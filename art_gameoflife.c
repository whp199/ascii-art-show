#include "art.h"
#include <stdlib.h>
#include <string.h>

static int *world;
static int *next_world;

void gol_init(int width, int height, ColorPalette* palette) {
    world = malloc(width * height * sizeof(int));
    next_world = malloc(width * height * sizeof(int));
    for (int i = 0; i < width * height; i++) {
        world[i] = (rand() % 4 == 0); // 25% chance of being alive
    }
}

void gol_destroy() {
    free(world);
    free(next_world);
}

void gol_update(double progress, double time_elapsed) {
    (void)progress; (void)time_elapsed;
    int width = get_buffer()->width;
    int height = get_buffer()->height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int neighbors = 0;
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (i == 0 && j == 0) continue;
                    int nx = (x + j + width) % width;
                    int ny = (y + i + height) % height;
                    neighbors += world[ny * width + nx];
                }
            }
            int current_cell = world[y * width + x];
            if (current_cell && (neighbors < 2 || neighbors > 3)) {
                next_world[y * width + x] = 0;
            } else if (!current_cell && neighbors == 3) {
                next_world[y * width + x] = 1;
            } else {
                next_world[y * width + x] = current_cell;
            }
        }
    }
    memcpy(world, next_world, width * height * sizeof(int));
}

void gol_draw(ScreenBuffer *buffer, ColorPalette* palette) {
    for (int i = 0; i < buffer->width * buffer->height; i++) {
        if (world[i]) {
            buffer_draw_char(i % buffer->width, i / buffer->width, '#', palette->colors[0], (Color){0,0,0});
        }
    }
}

ArtModule get_gameoflife_module() {
    return (ArtModule){
        .name = "game-of-life",
        .description = "Conway's Game of Life cellular automaton",
        .init = gol_init,
        .update = gol_update,
        .draw = gol_draw,
        .destroy = gol_destroy,
    };
}
