/**
 * main.c
 *
 * Main entry point for the advanced ASCII art slideshow.
 * Handles command-line arguments, the main application loop,
 * user input, and managing the art modules.
 */

// Define the POSIX source to get clock_gettime and nanosleep declarations
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#include "terminal.h"
#include "buffer.h"
#include "art.h"
#include "art_image.h"
#include "config.h"
#include "art_mtg.h"
#include "art_mtg_sixel.h"

// --- Main Application State ---
static int slide_duration = 20;
static int target_fps = 25;
static int show_info_hud = 0;
static int is_paused = 0;

// --- Art Module Registry ---
// Forward declarations of the art module getters
ArtModule get_mandelbrot_module();
ArtModule get_plasma_module();
ArtModule get_starfield_module();
ArtModule get_matrix_module();
ArtModule get_gameoflife_module();
ArtModule get_cube_module();
ArtModule get_clock_module();
ArtModule get_image_module();
ArtModule get_mtg_module();
ArtModule get_mtg_sixel_module();

// We declare the array here, but initialize it in main()
static ArtModule art_modules[9];
const int num_art_modules = sizeof(art_modules) / sizeof(ArtModule);

// --- Function Prototypes ---
void print_usage(const char *prog_name);
void list_modules();
void shuffle_modules();
void populate_modules();
void draw_hud(double time_left, int current_module_index, double fps);
int handle_input(int current_index);

int main(int argc, char **argv) {
    Configuration config = { .duration = 20, .fps = 25 };
    strcpy(config.palette, "default");
    load_config(&config);

    slide_duration = config.duration;
    target_fps = config.fps;

    // Populate the art modules array now that we are in a function
    populate_modules();

    int start_with_index = 0;
    int randomize_order = 0;

    // --- Command-line Argument Parsing ---
    int opt;
    static struct option long_options[] = {
        {"duration",    required_argument, 0, 'd'},
        {"fps",         required_argument, 0, 'f'},
        {"list",        no_argument,       0, 'l'},
        {"start-with",  required_argument, 0, 's'},
        {"random",      no_argument,       0, 'r'},
        {"image",       required_argument, 0, 'i'},
        {"palette",     required_argument, 0, 'p'},
        {"single",      no_argument,       0, 'S'},
        {"help",        no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    int single_mode = 0;

    while ((opt = getopt_long(argc, argv, "d:f:ls:ri:p:Sh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'd': slide_duration = atoi(optarg); break;
            case 'f': target_fps = atoi(optarg); break;
            case 'l': list_modules(); return 0;
            case 's':
                for(int i = 0; i < num_art_modules; i++) {
                    if (strcmp(art_modules[i].name, optarg) == 0) {
                        start_with_index = i;
                        break;
                    }
                }
                break;
            case 'i': image_set_path(optarg); break;
            case 'p': strncpy(config.palette, optarg, sizeof(config.palette) - 1); break;
            case 'S': single_mode = 1; break;
            case 'r': randomize_order = 1; break;
            case 'h': print_usage(argv[0]); return 0;
            default: print_usage(argv[0]); return 1;
        }
    }

    srand(time(NULL));
    if (randomize_order) {
        shuffle_modules();
    }

    // --- Terminal and Buffer Setup ---
    setup_terminal();
    if (!init_buffer(term_get_width(), term_get_height())) {
        cleanup_terminal();
        fprintf(stderr, "Failed to initialize screen buffer.\n");
        return 1;
    }

    // --- Main Loop ---
    int current_module_index = start_with_index;
    struct timespec last_frame_time;
    clock_gettime(CLOCK_MONOTONIC, &last_frame_time);

    while (1) {
        ArtModule *current_module = &art_modules[current_module_index];
        int is_sixel_module = (strcmp(current_module->name, "image") == 0 || strcmp(current_module->name, "mtg-sixel") == 0);
        int is_static_sixel = is_sixel_module; // For now, treat both as static
        int drawn = 0;

        if (current_module->init) {
            current_module->init(term_get_width(), term_get_height(), get_current_palette());
        }

        struct timespec slide_start_time;
        clock_gettime(CLOCK_MONOTONIC, &slide_start_time);

        while (1) {
            if (is_static_sixel && drawn) {
                // For static sixel images, we've drawn it once. Now just wait for input.
                int new_index = handle_input(current_module_index);
                if (new_index == -2) goto cleanup; // Quit
                if (new_index != current_module_index) {
                    current_module_index = new_index;
                    break;
                }
                // Sleep to prevent busy-waiting
                struct timespec sleep_time = {0, 10000000}; // 10ms
                nanosleep(&sleep_time, NULL);
                continue;
            }

            // Timing
            struct timespec current_time;
            clock_gettime(CLOCK_MONOTONIC, &current_time);
            double elapsed_slide_seconds = (current_time.tv_sec - slide_start_time.tv_sec) +
                                           (current_time.tv_nsec - slide_start_time.tv_nsec) / 1e9;
            double elapsed_frame_seconds = (current_time.tv_sec - last_frame_time.tv_sec) +
                                           (current_time.tv_nsec - last_frame_time.tv_nsec) / 1e9;

            if (term_has_resized()) {
                resize_buffer(term_get_width(), term_get_height());
                if (current_module->destroy) current_module->destroy();
                if (current_module->init) current_module->init(term_get_width(), term_get_height(), get_current_palette());
                drawn = 0; // Redraw after resize
            }

            int new_index = handle_input(current_module_index);
            if (new_index == -2) goto cleanup; // Quit
            if (new_index != current_module_index) {
                current_module_index = new_index;
                break;
            }

            // Update and Draw
            if (!is_paused) {
                double progress = elapsed_slide_seconds / slide_duration;
                if (current_module->update) {
                    current_module->update(progress, elapsed_slide_seconds);
                }
            }

            if (!is_sixel_module) {
                buffer_clear();
            }

            if (current_module->draw) {
                current_module->draw(get_buffer(), get_current_palette());
                if (is_static_sixel) {
                    drawn = 1;
                }
            }

            if (show_info_hud) {
                draw_hud(slide_duration - elapsed_slide_seconds, current_module_index, 1.0/elapsed_frame_seconds);
            }

            if (!is_sixel_module) {
                buffer_flush();
            }
            last_frame_time = current_time;

            // Check for next slide
            if (!single_mode && elapsed_slide_seconds >= slide_duration) {
                current_module_index = (current_module_index + 1) % num_art_modules;
                break;
            }

            // Use nanosleep instead of the obsolete usleep
            struct timespec sleep_time;
            long long sleep_ns = (1000000000LL / target_fps);
            sleep_time.tv_sec = sleep_ns / 1000000000LL;
            sleep_time.tv_nsec = sleep_ns % 1000000000LL;
            nanosleep(&sleep_time, NULL);
        }

        if (current_module->destroy) {
            current_module->destroy();
        }
    }

cleanup:
    destroy_buffer();
    cleanup_terminal();
    return 0;
}

void populate_modules() {
    art_modules[0] = get_mandelbrot_module();
    art_modules[1] = get_plasma_module();
    art_modules[2] = get_starfield_module();
    art_modules[3] = get_matrix_module();
    art_modules[4] = get_gameoflife_module();
    art_modules[5] = get_cube_module();
    art_modules[6] = get_clock_module();
    art_modules[7] = get_image_module();
    if (is_sixel_supported()) {
        art_modules[8] = get_mtg_sixel_module();
    } else {
        art_modules[8] = get_mtg_module();
    }
}

void print_usage(const char *prog_name) {
    printf("Usage: %s [options]\n", prog_name);
    printf("Options:\n");
    printf("  -d, --duration <secs>    Set slide duration (default: %d)\n", slide_duration);
    printf("  -f, --fps <num>          Set target FPS (default: %d)\n", target_fps);
    printf("  -l, --list               List available art modules and exit\n");
    printf("  -s, --start-with <name>  Start with a specific module\n");
    printf("  -r, --random             Randomize the order of modules\n");
    printf("  -h, --help               Show this help message\n");
}

void list_modules() {
    printf("Available art modules:\n");
    for (int i = 0; i < num_art_modules; i++) {
        printf("  - %s: %s\n", art_modules[i].name, art_modules[i].description);
    }
}

void shuffle_modules() {
    for (int i = num_art_modules - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        ArtModule temp = art_modules[i];
        art_modules[i] = art_modules[j];
        art_modules[j] = temp;
    }
}

void draw_hud(double time_left, int current_module_index, double fps) {
    char hud_text[256];
    snprintf(hud_text, sizeof(hud_text),
        "| %s | Time: %.1fs | FPS: %.1f | [P]ause [N]ext [B]ack [I]nfo [Q]uit |",
        art_modules[current_module_index].name,
        time_left < 0 ? 0 : time_left,
        fps);
    buffer_draw_text(1, 1, hud_text, (Color){255, 255, 255}, (Color){50, 50, 50});
}

int handle_input(int current_index) {
    int c = term_get_key();
    if (c != -1) {
        if (art_modules[current_index].handle_input) {
            art_modules[current_index].handle_input(c);
        }
    }
    switch (c) {
        case 'q': return -2; // Quit signal
        case 'p': is_paused = !is_paused; break;
        case 'i': show_info_hud = !show_info_hud; break;
        case 'n':
        case KEY_RIGHT:
             return (current_index + 1) % num_art_modules;
        case 'b':
        case KEY_LEFT:
             return (current_index - 1 + num_art_modules) % num_art_modules;
    }
    return current_index; // No change
}