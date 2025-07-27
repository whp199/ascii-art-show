#include "art.h"
#include <stdlib.h>

#define MAX_DROPS 200

typedef struct { int x; float y; int speed; int len; } Drop;
static Drop drops[MAX_DROPS];
static int num_drops;
static Color current_color;

static double lerp(double a, double b, double t) { return a + t * (b - a); }

void matrix_init(int width, int height) {
    num_drops = width < MAX_DROPS ? width : MAX_DROPS;
    for (int i = 0; i < num_drops; i++) {
        drops[i].x = rand() % width;
        drops[i].y = rand() % (height * 2) - height;
        drops[i].speed = rand() % 5 + 2;
        drops[i].len = rand() % (height / 2) + 5;
    }
}

void matrix_update(double progress, double time_elapsed) {
    (void)time_elapsed;
    int width = get_buffer()->width;
    int height = get_buffer()->height;

    current_color.r = (unsigned char)lerp(0, 80, progress);
    current_color.g = (unsigned char)lerp(255, 120, progress);
    current_color.b = (unsigned char)lerp(0, 255, progress);

    for (int i = 0; i < num_drops; i++) {
        drops[i].y += drops[i].speed / 20.0f;
        if (drops[i].y - drops[i].len > height) {
            drops[i].y = (float)(rand() % (height / 4));
            drops[i].x = rand() % width;
        }
    }
}

void matrix_draw(ScreenBuffer *buffer) {
    // Dim the screen
    for(int i = 0; i < buffer->width * buffer->height; i++) {
        if (rand() % 10 > 7) buffer->cells[i].character = ' ';
    }

    for (int i = 0; i < num_drops; i++) {
        int head_y = (int)drops[i].y;
        for (int j = 0; j < drops[i].len; j++) {
            int y = head_y - j;
            if (y >= 0 && y < buffer->height && drops[i].x < buffer->width) {
                Color c = current_color;
                c.g = (unsigned char)((float)c.g * (1.0f - (float)j / drops[i].len));
                 buffer_draw_char(drops[i].x, y, ' ' + (rand() % 94), c, (Color){0,0,0});
            }
        }
        if (head_y >= 0 && head_y < buffer->height && drops[i].x < buffer->width) {
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
