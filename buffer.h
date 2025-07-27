#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>

// Simple RGB color struct
typedef struct {
    unsigned char r, g, b;
} Color;

// A single cell on the screen
typedef struct {
    char character;
    Color fg;
    Color bg;
    int dirty; // Flag to check if this cell needs redrawing
} ScreenCell;

// Screen buffer struct
typedef struct {
    int width;
    int height;
    ScreenCell *cells;
} ScreenBuffer;

// Initialize the screen buffer system
int init_buffer(int width, int height);

// Resize the buffers (e.g., on terminal resize)
void resize_buffer(int new_width, int new_height);

// Free all resources used by the buffer system
void destroy_buffer();

// Clear the current drawing buffer (fill with spaces)
void buffer_clear();

// Flush the changes from the current buffer to the terminal
void buffer_flush();

// Draw a single character to the buffer
void buffer_draw_char(int x, int y, char c, Color fg, Color bg);

// Draw a string of text to the buffer
void buffer_draw_text(int x, int y, const char* text, Color fg, Color bg);

// Draw a line to the buffer using Bresenham's algorithm
void buffer_draw_line(int x0, int y0, int x1, int y1, char ch, Color c);

// Get a pointer to the main screen buffer
ScreenBuffer* get_buffer();

int buffer_get_width(ScreenBuffer *buffer);
int buffer_get_height(ScreenBuffer *buffer);
void buffer_set_char(ScreenBuffer *buffer, int x, int y, char character, Color fg, Color bg);

#endif // BUFFER_H