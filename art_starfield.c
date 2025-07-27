#include "art.h"
#include <stdlib.h>

#define NUM_STARS 500

typedef struct { float x, y, z; } Star;
static Star stars[NUM_STARS];
static float speed;

void starfield_init(int width, int height) {
    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].x = (rand() % width) - width / 2;
        stars[i].y = (rand() % height) - height / 2;
        stars[i].z = rand() % width;
    }
}

void starfield_update(double progress, double time_elapsed) {
    (void)time_elapsed;
    speed = 0.5 + progress * 2.5;
    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].z -= speed;
        if (stars[i].z <= 0) {
            stars[i].x = (rand() % get_buffer()->width) - get_buffer()->width / 2;
            stars[i].y = (rand() % get_buffer()->height) - get_buffer()->height / 2;
            stars[i].z = get_buffer()->width;
        }
    }
}

void starfield_draw(ScreenBuffer *buffer) {
    for (int i = 0; i < NUM_STARS; i++) {
        if (stars[i].z > 0) {
            int sx = (int)(stars[i].x / (stars[i].z * 0.005)) + buffer->width / 2;
            int sy = (int)(stars[i].y / (stars[i].z * 0.005)) + buffer->height / 2;

            if (sx >= 0 && sx < buffer->width && sy >= 0 && sy < buffer->height) {
                float dist_ratio = stars[i].z / buffer->width;
                char character = '.'; Color c = {100, 100, 100};
                if (dist_ratio < 0.2) { character = '@'; c = (Color){255, 255, 255}; }
                else if (dist_ratio < 0.5) { character = '*'; c = (Color){200, 200, 200}; }
                else if (dist_ratio < 0.8) { character = '+'; c = (Color){150, 150, 150}; }
                buffer_draw_char(sx, sy, character, c, (Color){0,0,0});
            }
        }
    }
}

ArtModule get_starfield_module() {
    return (ArtModule){
        .name = "starfield",
        .description = "Accelerating through a 3D starfield",
        .init = starfield_init,
        .update = starfield_update,
        .draw = starfield_draw,
        .destroy = NULL,
    };
}
