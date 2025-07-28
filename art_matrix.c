#include "art.h"
#include <stdlib.h>
#include <math.h> // For sinf, cosf, fminf, powf

#define MAX_DROPS 200

typedef struct { int x; float y; int speed; int len; } Drop;
static Drop drops[MAX_DROPS];
static int num_drops;
static double current_time = 0.0;

void matrix_init(int width, int height, ColorPalette* palette) {
    (void)palette;
    // Increase the number of drops for a more saturated effect
    num_drops = (int)(width * 0.75f) < MAX_DROPS ? (int)(width * 0.75f) : MAX_DROPS;
    for (int i = 0; i < num_drops; i++) {
        drops[i].x = rand() % width;
        drops[i].y = (float)(rand() % (height * 2) - height);
        drops[i].speed = rand() % 5 + 2;
        // Make drops longer for a more persistent effect
        drops[i].len = rand() % (height / 2) + (height / 4);
    }
}

void matrix_update(double progress, double time_elapsed) {
    (void)progress; // No longer tied to slide progress
    current_time = time_elapsed;

    int width = get_buffer()->width;
    int height = get_buffer()->height;

    for (int i = 0; i < num_drops; i++) {
        drops[i].y += drops[i].speed / 20.0f;
        // When a drop is finished, reset it to the top of the screen
        if (drops[i].y - drops[i].len > height) {
            drops[i].y = 0;
            drops[i].x = rand() % width;
        }
    }
}

void matrix_draw(ScreenBuffer *buffer, ColorPalette* palette) {
    // Dim the screen using the old-style random blanking
    for(int i = 0; i < buffer->width * buffer->height; i++) {
        if (rand() % 10 > 7) buffer->cells[i].character = ' ';
    }

    for (int i = 0; i < num_drops; i++) {
        // Use sine waves based on position and time to create a pulsing color
        float pulse = (sinf(drops[i].x * 0.1f + current_time * 2.0f) +
                       cosf(drops[i].y * 0.1f + current_time)) / 2.0f; // range -1 to 1

        // Map the pulse to a color from the palette
        int color_index = (int)(((pulse + 1.0f) / 2.0f) * (MAX_PALETTE_COLORS - 1));
        if (color_index < 0) color_index = 0;
        if (color_index >= MAX_PALETTE_COLORS) color_index = MAX_PALETTE_COLORS - 1;
        Color base_color = palette->colors[color_index];

        int head_y = (int)drops[i].y;
        for (int j = 0; j < drops[i].len; j++) {
            int y = head_y - j;
            if (y >= 0 && y < buffer->height && drops[i].x >= 0 && drops[i].x < buffer->width) {
                // Keep the nice fade effect, but apply it to the new pulsing color
                float fade = (float)(drops[i].len - j) / drops[i].len;
                fade = powf(fade, 1.5f);

                Color c = {
                    .r = (unsigned char)(base_color.r * fade),
                    .g = (unsigned char)(base_color.g * fade),
                    .b = (unsigned char)(base_color.b * fade)
                };
                buffer_draw_char(drops[i].x, y, ' ' + (rand() % 94), c, (Color){0,0,0});
            }
        }

        // The head of the drop is the classic bright white-green color
        if (head_y >= 0 && head_y < buffer->height && drops[i].x >= 0 && drops[i].x < buffer->width) {
            buffer_draw_char(drops[i].x, head_y, ' ' + (rand() % 94), (Color){200, 255, 200}, (Color){0,0,0});
        }
    }
}

ArtModule get_matrix_module() {
    return (ArtModule){
        .name = "matrix",
        .description = "The classic digital rain effect",
        .init = matrix_init,
        .update = matrix_update,
        .draw = matrix_draw,
        .destroy = NULL,
    };
}
