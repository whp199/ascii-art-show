#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "art.h"
#include "buffer.h"
#include "terminal.h"
#include "art_mtg_sixel.h"

static const char *image_path = "image.png"; // Default image path

void image_set_path(const char *path) {
    image_path = path;
}

void image_init(int width, int height, ColorPalette* palette) {
    (void)width; // Unused
    (void)height; // Unused
    (void)palette; // Unused
}

void image_draw(ScreenBuffer *buffer, ColorPalette* palette) {
    (void)buffer; // Unused
    (void)palette;
    if (!is_sixel_supported()) {
        buffer_draw_text(1, 1, "Error: Sixel is not supported in this terminal.", (Color){255, 0, 0}, (Color){0, 0, 0});
        return;
    }

    // Clear the screen before drawing the Sixel image
    printf("\e[2J\e[H");
    fflush(stdout);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Child process
        execlp("img2sixel", "img2sixel", image_path, NULL);
        // If execlp returns, it must have failed
        perror("execlp");
        exit(1);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
}

void image_destroy() {
    // No-op
}

ArtModule get_image_module() {
    return (ArtModule){
        .name = "image",
        .description = "Displays a static image using Sixel. Use the --image option to specify the path.",
        .init = image_init,
        .update = NULL,
        .draw = image_draw,
        .destroy = image_destroy,
    };
}