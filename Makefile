# Makefile for the ASCII Art Slideshow

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -I/usr/include/sixel
LDFLAGS = -lm -lcurl -lsixel -lsixel

# Source files
SRCS = main.c \
       terminal.c \
       buffer.c \
       art_mandelbrot.c \
       art_plasma.c \
       art_starfield.c \
       art_matrix.c \
       art_gameoflife.c \
       art_cube.c \
       art_clock.c \
       art_image.c \
       config.c \
       ini.c \
       art_mtg.c \
       art_mtg_sixel.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = ascii-art-show

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
