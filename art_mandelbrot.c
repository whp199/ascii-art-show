#include "art.h"
#include <math.h>

static double current_re, current_im, range;

static double lerp(double a, double b, double t) { return a + t * (b - a); }

void mandelbrot_update(double progress, double time_elapsed) {
    (void)time_elapsed;
    const double start_re = -0.5, end_re = -0.743643887037151;
    const double start_im = 0.0, end_im = 0.131825904205330;
    const double start_range = 4.0, end_range = 0.000002;
    double t = progress * progress * (3.0 - 2.0 * progress);

    current_re = lerp(start_re, end_re, t);
    current_im = lerp(start_im, end_im, t);
    range = lerp(start_range, end_range, t);
}

void mandelbrot_draw(ScreenBuffer *buffer) {
    int max_iter = 128;
    for (int row = 0; row < buffer->height; row++) {
        for (int col = 0; col < buffer->width; col++) {
            double c_re = current_re + (col - buffer->width / 2.0) * range / buffer->width;
            double c_im = current_im + (row - buffer->height / 2.0) * range / buffer->width * 0.5; // Aspect ratio
            double x = 0, y = 0;
            int iteration = 0;
            while (x * x + y * y <= 4 && iteration < max_iter) {
                double x_new = x * x - y * y + c_re;
                y = 2 * x * y + c_im;
                x = x_new;
                iteration++;
            }
            if (iteration < max_iter) {
                int color_val = (iteration * 12) % 230 + 16;
                Color c = {(unsigned char)(sin(0.1 * color_val) * 127 + 128),
                           (unsigned char)(sin(0.1 * color_val + 2) * 127 + 128),
                           (unsigned char)(sin(0.1 * color_val + 4) * 127 + 128)};
                buffer_draw_char(col, row, '.', c, (Color){0,0,0});
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
    };
}
