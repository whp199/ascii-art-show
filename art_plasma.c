#include "art.h"
#include <math.h>
#include <string.h>

static double g_time_elapsed;
static double g_progress;

void plasma_update(double progress, double time_elapsed) {
    g_progress = progress;
    g_time_elapsed = time_elapsed;
}

void plasma_draw(ScreenBuffer *buffer, ColorPalette* palette) {
    const char* charset = " .:-=+*#%@";
    int charset_size = strlen(charset);
    int color_shift = (int)(g_progress * 200.0);

    for (int y = 0; y < buffer->height; y++) {
        for (int x = 0; x < buffer->width; x++) {
            double val = sin(x / 16.0 + g_time_elapsed) +
                         sin(y / 8.0 - g_time_elapsed * 1.5) +
                         sin(sqrt((double)(x - buffer->width / 2) * (x - buffer->width/2) + (y - buffer->height / 2) * (y - buffer->height / 2)) / 8.0 + g_time_elapsed);

            float t = (val + 3.0) / 6.0;
            Color c = get_palette_color(palette, t + g_progress);

            int char_index = (int)((val + 3.0) / 6.0 * charset_size);
            char_index = fmax(0, fmin(charset_size - 1, char_index));
            buffer_draw_char(x, y, charset[char_index], c, (Color){0,0,0});
        }
    }
}

ArtModule get_plasma_module() {
    return (ArtModule){
        .name = "plasma",
        .description = "Flowing clouds of colorful plasma",
        .init = NULL,
        .update = plasma_update,
        .draw = plasma_draw,
        .destroy = NULL,
    };
}
