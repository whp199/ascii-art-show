#include "art.h"
#include "buffer.h"
#include <curl/curl.h>
#include "json.h"
#include "stb_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static stbi_uc *image_data = NULL;
static int image_width = 0;
static int image_height = 0;
static int image_channels = 0;

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

static void mtg_fetch_and_display();

void mtg_init(int width, int height, ColorPalette* palette) {
    (void)width; (void)height; (void)palette;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    mtg_fetch_and_display();
}

void mtg_update(double progress, double time_elapsed) {
    (void)progress;
    // Every 5 seconds, fetch a new card
    if (fmod(time_elapsed, 5.0) < 0.1) {
        mtg_fetch_and_display();
    }
}

void mtg_draw(ScreenBuffer *buffer, ColorPalette* palette) {
    (void)palette;
    if (image_data == NULL) {
        buffer_draw_text(1, 1, "Loading...", (Color){255, 255, 255}, (Color){0, 0, 0});
        return;
    }

    int buffer_w = buffer_get_width(buffer);
    int buffer_h = buffer_get_height(buffer);

    for (int y = 0; y < buffer_h; y++) {
        for (int x = 0; x < buffer_w; x++) {
            int img_x = (int)((float)x / buffer_w * image_width);
            int img_y = (int)((float)y / buffer_h * image_height);

            int index = (img_y * image_width + img_x) * 3;
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

void mtg_destroy() {
    if (image_data) {
        stbi_image_free(image_data);
        image_data = NULL;
    }
    curl_global_cleanup();
}

static void mtg_fetch_and_display() {
    CURL *curl_handle;
    CURLcode res;

    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_handle = curl_easy_init();
    if(curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, "https://api.scryfall.com/cards/random");
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl_handle);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            struct json_value_s* root = json_parse(chunk.memory, chunk.size);
            if (root) {
                struct json_object_s* object = (struct json_object_s*)root->payload;
                struct json_value_s* image_uris = NULL;
                for (struct json_object_element_s* i = object->start; i != NULL; i = i->next) {
                    if (strcmp(i->name->string, "image_uris") == 0) {
                        image_uris = i->value;
                        break;
                    }
                }

                if (image_uris) {
                    struct json_object_s* image_uris_obj = (struct json_object_s*)image_uris->payload;
                    for (struct json_object_element_s* i = image_uris_obj->start; i != NULL; i = i->next) {
                        if (strcmp(i->name->string, "normal") == 0) {
                            struct json_string_s* image_url = (struct json_string_s*)i->value->payload;
                            
                            CURL *img_curl_handle;
                            struct MemoryStruct img_chunk;
                            img_chunk.memory = malloc(1);
                            img_chunk.size = 0;

                            img_curl_handle = curl_easy_init();
                            if(img_curl_handle) {
                                curl_easy_setopt(img_curl_handle, CURLOPT_URL, image_url->string);
                                curl_easy_setopt(img_curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
                                curl_easy_setopt(img_curl_handle, CURLOPT_WRITEDATA, (void *)&img_chunk);
                                curl_easy_setopt(img_curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

                                res = curl_easy_perform(img_curl_handle);

                                if(res != CURLE_OK) {
                                    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                                } else {
                                    if (image_data) {
                                        stbi_image_free(image_data);
                                    }
                                    image_data = stbi_load_from_memory(img_chunk.memory, img_chunk.size, &image_width, &image_height, &image_channels, 3);
                                }
                                curl_easy_cleanup(img_curl_handle);
                                free(img_chunk.memory);
                            }
                            break;
                        }
                    }
                }
                free(root);
            }
        }
        curl_easy_cleanup(curl_handle);
        free(chunk.memory);
    }
}

ArtModule get_mtg_module() {
    return (ArtModule){
        .name = "mtg",
        .description = "Displays random Magic: The Gathering cards.",
        .init = mtg_init,
        .update = mtg_update,
        .draw = mtg_draw,
        .destroy = mtg_destroy,
        .handle_input = NULL,
    };
}