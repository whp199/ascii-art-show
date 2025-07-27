#include "config.h"
#define INI_IMPLEMENTATION
#include "ini.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Default color palette
static ColorPalette palettes[] = {
    {
        .name = "default",
        .colors = {
            {255, 0, 0}, {255, 127, 0}, {255, 255, 0}, {0, 255, 0}, {0, 0, 255}, {75, 0, 130}, {148, 0, 211}
        }
    },
    {
        .name = "pastel",
        .colors = {
            {255, 179, 186}, {255, 204, 186}, {255, 229, 186}, {255, 255, 186}, {229, 255, 186}, {204, 255, 186}, {179, 255, 186}, {186, 255, 201}, {186, 255, 229}, {186, 225, 255}, {186, 201, 255}, {204, 186, 255}, {229, 186, 255}, {255, 186, 229}, {255, 186, 204}
        }
    },
    {
        .name = "vaporwave",
        .colors = {
            {255, 110, 199}, {255, 110, 229}, {204, 110, 255}, {14, 200, 240}, {14, 220, 240}, {14, 240, 220}, {14, 240, 180}
        }
    },
    {
        .name = "rainbow",
        .colors = {
            {255, 0, 0}, {255, 127, 0}, {255, 255, 0}, {0, 255, 0}, {0, 0, 255}, {75, 0, 130}, {148, 0, 211}
        }
    }
};

static int num_palettes = sizeof(palettes) / sizeof(ColorPalette);
static int current_palette_index = 0;

static int handler(void* user, const char* section, const char* name, const char* value) {
    Configuration* pconfig = (Configuration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    if (MATCH("slideshow", "duration")) {
        pconfig->duration = atoi(value);
    } else if (MATCH("slideshow", "fps")) {
        pconfig->fps = atoi(value);
    } else if (MATCH("slideshow", "palette")) {
        for (int i = 0; i < num_palettes; i++) {
            if (strcmp(palettes[i].name, value) == 0) {
                current_palette_index = i;
                break;
            }
        }
    } else {
        return 0; /* unknown section/name, error */
    }
    return 1;
}

int load_config(Configuration* config) {
    char path[1024];
    const char* home = getenv("HOME");
    if (home) {
        snprintf(path, sizeof(path), "%s/.config/ascii-art-show/config", home);
    } else {
        return -1; // HOME not set
    }

    if (ini_parse(path, handler, config) < 0) {
        // Could not open file, but that's ok, we'll use defaults
        return 1;
    }

    return 0;
}

ColorPalette* get_current_palette() {
    return &palettes[current_palette_index];
}

Color get_palette_color(ColorPalette* palette, float t) {
    t = fmodf(t, 1.0f);
    if (t < 0) t += 1.0f;

    int num_colors = 0;
    while (num_colors < MAX_PALETTE_COLORS && (palette->colors[num_colors].r != 0 || palette->colors[num_colors].g != 0 || palette->colors[num_colors].b != 0)) {
        num_colors++;
    }

    if (num_colors == 0) {
        return (Color){0, 0, 0};
    }
    if (num_colors == 1) {
        return palette->colors[0];
    }

    float scaled_t = t * (num_colors - 1);
    int index1 = (int)scaled_t;
    int index2 = (index1 + 1) % num_colors;
    float fraction = scaled_t - index1;

    Color c1 = palette->colors[index1];
    Color c2 = palette->colors[index2];

    Color result;
    result.r = (unsigned char)((1.0f - fraction) * c1.r + fraction * c2.r);
    result.g = (unsigned char)((1.0f - fraction) * c1.g + fraction * c2.g);
    result.b = (unsigned char)((1.0f - fraction) * c1.b + fraction * c2.b);

    return result;
}
