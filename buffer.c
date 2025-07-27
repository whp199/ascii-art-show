#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ScreenBuffer current_buffer;
static ScreenBuffer prev_buffer;

int init_buffer(int width, int height) {
    current_buffer.width = width;
    current_buffer.height = height;
    current_buffer.cells = malloc(width * height * sizeof(ScreenCell));
    if (!current_buffer.cells) return 0;

    prev_buffer.width = width;
    prev_buffer.height = height;
    prev_buffer.cells = malloc(width * height * sizeof(ScreenCell));
    if (!prev_buffer.cells) {
        free(current_buffer.cells);
        return 0;
    }

    // Initialize previous buffer to a known state
    for (int i = 0; i < width * height; i++) {
        prev_buffer.cells[i].character = ' ';
        prev_buffer.cells[i].dirty = 1; // Force full redraw on first flush
    }
    return 1;
}

void resize_buffer(int new_width, int new_height) {
    destroy_buffer();
    init_buffer(new_width, new_height);
}

void destroy_buffer() {
    free(current_buffer.cells);
    free(prev_buffer.cells);
}

void buffer_clear() {
    for (int i = 0; i < current_buffer.width * current_buffer.height; i++) {
        current_buffer.cells[i].character = ' ';
        current_buffer.cells[i].fg = (Color){255, 255, 255};
        current_buffer.cells[i].bg = (Color){0, 0, 0};
    }
}

void buffer_flush() {
    printf("\x1b[?25l"); // Hide cursor

    Color last_fg = {-1, -1, -1};
    Color last_bg = {-1, -1, -1};

    for (int y = 0; y < current_buffer.height; y++) {
        for (int x = 0; x < current_buffer.width; x++) {
            ScreenCell *current = &current_buffer.cells[y * current_buffer.width + x];
            ScreenCell *prev = &prev_buffer.cells[y * prev_buffer.width + x];

            // Only redraw if the cell has changed
            if (current->character != prev->character ||
                memcmp(&current->fg, &prev->fg, sizeof(Color)) != 0 ||
                memcmp(&current->bg, &prev->bg, sizeof(Color)) != 0)
            {
                // Move cursor to the correct position
                printf("\x1b[%d;%dH", y + 1, x + 1);

                // Set foreground color if it changed
                if (memcmp(&current->fg, &last_fg, sizeof(Color)) != 0) {
                    printf("\x1b[38;2;%d;%d;%dm", current->fg.r, current->fg.g, current->fg.b);
                    last_fg = current->fg;
                }
                // Set background color if it changed
                if (memcmp(&current->bg, &last_bg, sizeof(Color)) != 0) {
                    printf("\x1b[48;2;%d;%d;%dm", current->bg.r, current->bg.g, current->bg.b);
                    last_bg = current->bg;
                }

                putchar(current->character);
            }
        }
    }
    printf("\x1b[0m"); // Reset attributes
    fflush(stdout);

    // Copy current buffer to previous buffer for the next frame's comparison
    memcpy(prev_buffer.cells, current_buffer.cells, current_buffer.width * current_buffer.height * sizeof(ScreenCell));
}

void buffer_draw_char(int x, int y, char c, Color fg, Color bg) {
    if (x >= 0 && x < current_buffer.width && y >= 0 && y < current_buffer.height) {
        int index = y * current_buffer.width + x;
        current_buffer.cells[index].character = c;
        current_buffer.cells[index].fg = fg;
        current_buffer.cells[index].bg = bg;
    }
}

void buffer_draw_text(int x, int y, const char* text, Color fg, Color bg) {
    int i = 0;
    while(text[i] != '\0') {
        buffer_draw_char(x + i, y, text[i], fg, bg);
        i++;
    }
}

// Implementation of Bresenham's line algorithm
void buffer_draw_line(int x0, int y0, int x1, int y1, char ch, Color c) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;) {
        buffer_draw_char(x0, y0, ch, c, (Color){0,0,0});
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}


ScreenBuffer* get_buffer() {
    return &current_buffer;
}

int buffer_get_width(ScreenBuffer *buffer) {
    return buffer->width;
}

int buffer_get_height(ScreenBuffer *buffer) {
    return buffer->height;
}

void buffer_set_char(ScreenBuffer *buffer, int x, int y, char character, Color fg, Color bg) {
    if (x >= 0 && x < buffer->width && y >= 0 && y < buffer->height) {
        int index = y * buffer->width + x;
        buffer->cells[index].character = character;
        buffer->cells[index].fg = fg;
        buffer->cells[index].bg = bg;
    }
}
