#include "art.h"
#include "terminal.h"
#include <math.h>

static double current_re = -0.5, current_im = 0.0, range = 4.0;

void mandelbrot_handle_input(int key) {
    switch (key) {
        case 'w':
            current_im -= range * 0.1;
            break;
        case 's':
            current_im += range * 0.1;
            break;
        case 'a':
            current_re -= range * 0.1;
            break;
        case 'd':
            current_re += range * 0.1;
            break;
        case '=': // Zoom in
        case '+':
            range *= 0.9;
            break;
        case '-': // Zoom out
            range *= 1.1;
            break;
    }
}

void mandelbrot_update(double progress, double time_elapsed) {
    (void)progress; (void)time_elapsed; // No-op
}

void mandelbrot_draw(ScreenBuffer *buffer, ColorPalette* palette) {
    int max_iter = 256; // Increased for more detail
    for (int row = 0; row < buffer->height; row++) {
        for (int col = 0; col < buffer->width; col++) {
            // Map pixel to the complex plane, adjusting for aspect ratio
            double c_re = current_re + (col - buffer->width / 2.0) * range / buffer->width;
            double c_im = current_im + (row - buffer->height / 2.0) * range / buffer->width * 0.5;

            double x = 0, y = 0;
            int iteration = 0;
            // Use a larger escape radius for the smooth coloring algorithm
            while (x * x + y * y <= (1 << 16) && iteration < max_iter) {
                double x_new = x * x - y * y + c_re;
                y = 2 * x * y + c_im;
                x = x_new;
                iteration++;
            }

            if (iteration < max_iter) {
                // Smooth coloring algorithm
                double log_zn = log(x * x + y * y) / 2.0;
                double nu = log(log_zn / log(2.0)) / log(2.0);
                
                // Scale the smooth iteration value to the palette
                float t = (float)(iteration + 1.0 - nu) / (float)max_iter;

                // Cycle through the palette multiple times for more color variation
                Color c = get_palette_color(palette, fmodf(t * 10.0f, 1.0f));
                buffer_draw_char(col, row, '#', c, (Color){0,0,0});
            } else {
                // Points inside the set are black
                buffer_draw_char(col, row, ' ', (Color){0,0,0}, (Color){0,0,0});
            }
        }
    }
}

ArtModule get_mandelbrot_module() {
    return (ArtModule){
        .name = "mandelbrot",
        .description = "A journey into the Mandelbrot fractal set",
        .init = NULL,
        .update = mandelbrot_update,
        .draw = mandelbrot_draw,
        .destroy = NULL,
        .handle_input = mandelbrot_handle_input,
    };
}
