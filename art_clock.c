#include "art.h"
#include <time.h>
#include <math.h>

const float PI = 3.1415926535f;

void clock_draw(ScreenBuffer *buffer) {
    int center_x = buffer->width / 2;
    int center_y = buffer->height / 2;
    int radius = (buffer->height < buffer->width/2 ? buffer->height : buffer->width/2) / 2 - 2;
    if (radius < 1) radius = 1;

    // Draw face
    for (int i = 0; i < 12; i++) {
        float angle = (float)i / 12.0f * 2.0f * PI;
        int x = center_x + (int)(sinf(angle) * radius * 2); // *2 for aspect ratio
        int y = center_y - (int)(cosf(angle) * radius);
        buffer_draw_char(x, y, 'o', (Color){255, 255, 0}, (Color){0,0,0});
    }

    // Get time
    time_t raw_time;
    struct tm *time_info;
    time(&raw_time);
    time_info = localtime(&raw_time);

    // Hour hand
    float hour_angle = (time_info->tm_hour % 12 + time_info->tm_min / 60.0f) / 12.0f * 2.0f * PI;
    buffer_draw_line(center_x, center_y,
              center_x + (int)(sinf(hour_angle) * radius * 1.0),
              center_y - (int)(cosf(hour_angle) * radius * 0.5),
              '#', (Color){255, 100, 100});

    // Minute hand
    float min_angle = (time_info->tm_min + time_info->tm_sec / 60.0f) / 60.0f * 2.0f * PI;
    buffer_draw_line(center_x, center_y,
              center_x + (int)(sinf(min_angle) * radius * 1.8),
              center_y - (int)(cosf(min_angle) * radius * 0.9),
              '+', (Color){100, 255, 100});

    // Second hand
    float sec_angle = time_info->tm_sec / 60.0f * 2.0f * PI;
    buffer_draw_line(center_x, center_y,
              center_x + (int)(sinf(sec_angle) * radius * 1.9),
              center_y - (int)(cosf(sec_angle) * radius * 0.95),
              '.', (Color){150, 150, 255});
}

ArtModule get_clock_module() {
    return (ArtModule){
        .name = "clock",
        .description = "An analog-style ASCII clock showing system time",
        .init = NULL,
        .update = NULL,
        .draw = clock_draw,
        .destroy = NULL,
    };
}
