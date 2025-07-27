#ifndef ART_H
#define ART_H

#include "buffer.h"
#include "config.h"

// The interface for any art module
typedef struct {
    const char *name;
    const char *description;
    // Called once when the module becomes active
    void (*init)(int width, int height, ColorPalette* palette);
    // Called once per frame, if the animation evolves
    void (*update)(double progress, double time_elapsed);
    // Called once per frame to draw to the buffer
    void (*draw)(ScreenBuffer *buffer, ColorPalette* palette);
    // Called once when the module becomes inactive
    void (*destroy)();
} ArtModule;

#endif // ART_H