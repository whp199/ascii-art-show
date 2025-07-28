#include "art_mtg_sixel.h"
#include "buffer.h"
#include <sixel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/wait.h>
#include <curl/curl.h>
#include "json.h"

#define TMP_IMAGE_PATH "/tmp/mtg_card.png"

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
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

static void mtg_sixel_fetch_random_card() {
    CURL *curl_handle;
    CURLcode res;

    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
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
                        if (strcmp(i->name->string, "png") == 0) {
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
                                    FILE *fp = fopen(TMP_IMAGE_PATH, "wb");
                                    if (fp) {
                                        fwrite(img_chunk.memory, 1, img_chunk.size, fp);
                                        fclose(fp);
                                    }
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
    curl_global_cleanup();
}


void mtg_sixel_init(int width, int height, ColorPalette* palette) {
    (void)width; (void)height; (void)palette;
    mtg_sixel_fetch_random_card();
}

void mtg_sixel_update(double progress, double time_elapsed) {
    (void)progress; (void)time_elapsed;
}

void mtg_sixel_draw(ScreenBuffer *buffer, ColorPalette* palette) {
    (void)buffer; (void)palette;
    
    printf("\e[2J\e[H");
    fflush(stdout);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        execlp("img2sixel", "img2sixel", TMP_IMAGE_PATH, NULL);
        perror("execlp");
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}

void mtg_sixel_destroy() {
    remove(TMP_IMAGE_PATH);
}

int is_sixel_supported() {
    struct termios old_tio, new_tio;
    char response[32] = {0};
    int ret = 0;
    ssize_t bytes_read;

    if (tcgetattr(STDIN_FILENO, &old_tio) != 0) return 0;
    new_tio = old_tio;
    new_tio.c_lflag &= ~(ICANON | ECHO);
    new_tio.c_cc[VMIN] = 0;
    new_tio.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_tio) != 0) return 0;

    write(STDOUT_FILENO, "\x1b[c", 3);
    bytes_read = read(STDIN_FILENO, response, sizeof(response) - 1);
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);

    if (bytes_read > 0) {
        for (int i = 0; i < bytes_read; ++i) {
            if (response[i] == '4') {
                ret = 1;
                break;
            }
        }
    }
    return ret;
}

ArtModule get_mtg_sixel_module() {
    return (ArtModule){
        .name = "mtg-sixel",
        .description = "Displays random Magic: The Gathering cards using Sixel graphics.",
        .init = mtg_sixel_init,
        .update = NULL, // This is a static image for the duration of the slide
        .draw = mtg_sixel_draw,
        .destroy = mtg_sixel_destroy,
        .handle_input = NULL,
    };
}