#ifndef ART_H
#define ART_H

#include "buffer.h"

// The interface for any art module
typedef struct {
    const char *name;
    const char *description;
    // Called once when the module becomes active
    void (*init)(int width, int height);
    // Called once per frame, if the animation evolves
    void (*update)(double progress, double time_elapsed);
    // Called once per frame to draw to the buffer
    void (*draw)(ScreenBuffer *buffer);
    // Called once when the module becomes inactive
    void (*destroy)();
} ArtModule;

#endif // ART_H
