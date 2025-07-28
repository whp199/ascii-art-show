#include <stdio.h>
#include <sixel.h>

int main() {
    sixel_encoder_t *encoder = NULL;
    sixel_encoder_new(&encoder, NULL);
    sixel_encoder_encode(encoder, "image.png");
    sixel_encoder_unref(encoder);
    return 0;
}