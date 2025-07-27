
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "art.h"
#include "buffer.h"
#include "terminal.h"

static stbi_uc *image_data = NULL;
static int image_width = 0;
static int image_height = 0;
static int image_channels = 0;

static const char *image_path = "image.png"; // Default image path

void image_set_path(const char *path) {
    image_path = path;
}

void image_init(int width, int height) {
    (void)width; // Unused
    (void)height; // Unused
    image_data = stbi_load(image_path, &image_width, &image_height, &image_channels, 3);
    if (image_data == NULL) {
        fprintf(stderr, "Error loading image: %s\n", stbi_failure_reason());
    }
}

void image_draw(ScreenBuffer *buffer) {
    if (image_data == NULL) {
        buffer_draw_text(1, 1, "Error: Could not load image.", (Color){255, 0, 0}, (Color){0, 0, 0});
        return;
    }

    int buffer_w = buffer_get_width(buffer);
    int buffer_h = buffer_get_height(buffer);

    for (int y = 0; y < buffer_h; y++) {
        for (int x = 0; x < buffer_w; x++) {
            int img_x = (int)((float)x / buffer_w * image_width);
            int img_y = (int)((float)y / buffer_h * image_height);

            int index = (img_y * image_width + img_x) * 3; // 3 channels (RGB)
            if (index < image_width * image_height * 3) {
                Color color = {
                    image_data[index],
                    image_data[index + 1],
                    image_data[index + 2]
                };
                buffer_set_char(buffer, x, y, ' ', color, color);
            }
        }
    }
}

void image_destroy() {
    if (image_data) {
        stbi_image_free(image_data);
        image_data = NULL;
    }
}

ArtModule get_image_module() {
    return (ArtModule){
        .name = "image",
        .description = "Displays a static image. Use the --image option to specify the path.",
        .init = image_init,
        .update = NULL,
        .draw = image_draw,
        .destroy = image_destroy,
    };
}
