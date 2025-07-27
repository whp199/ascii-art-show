#ifndef CONFIG_H
#define CONFIG_H

#include "buffer.h"
#include <math.h>

#define MAX_PALETTE_COLORS 16

typedef struct {
    char name[32];
    Color colors[MAX_PALETTE_COLORS];
} ColorPalette;

typedef struct {
    int duration;
    int fps;
    char palette[32];
} Configuration;

int load_config(Configuration* config);
ColorPalette* get_current_palette();
Color get_palette_color(ColorPalette* palette, float t);

#endif // CONFIG_H
