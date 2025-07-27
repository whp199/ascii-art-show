# Advanced ASCII Art Slideshow

This is a terminal-based slideshow application that displays various animated and static ASCII art modules. It's written in C and is designed to be easily extensible with new art modules.

## Features

*   A variety of art modules, including:
    *   Mandelbrot Set
    *   Plasma Effect
    *   Starfield
    *   Matrix-style scrolling text
    *   Conway's Game of Life
    *   3D Spinning Cube
    *   Digital Clock
    *   Image Viewer
*   Command-line options to customize the slideshow.
*   Interactive controls to pause, navigate, and quit.
*   Dynamic resizing to fit the terminal window.

## Building the Project

To build the project, you'll need a C compiler (like `gcc`) and the `make` utility.

```bash
make
```

This will create an executable file named `ascii-art-show` in the project directory.

## Usage

To run the slideshow with the default settings, simply execute the program:

```bash
./ascii-art-show
```

### Command-line Options

You can customize the slideshow with the following command-line options:

| Option                | Short | Description                                           | Default |
| --------------------- | ----- | ----------------------------------------------------- | ------- |
| `--duration <secs>`   | `-d`  | Set the duration for each slide in seconds.           | 20      |
| `--fps <num>`         | `-f`  | Set the target frames per second for animations.      | 25      |
| `--list`              | `-l`  | List all available art modules and exit.              |         |
| `--start-with <name>` | `-s`  | Start the slideshow with a specific module.           |         |
| `--random`            | `-r`  | Randomize the order of the art modules.               |         |
| `--image <path>`      | `-i`  | Specify the path to an image for the `image` module.  |         |
| `--palette <name>`    | `-p`  | Choose a color palette (e.g., `default`, `pastel`).   | `default` |
| `--single`            | `-S`  | Display a single module without the slideshow.        |         |
| `--help`              | `-h`  | Display the help message and exit.                    |         |

### Configuration File

You can also configure the application using a configuration file located at `~/.config/ascii-art-show/config`. The file uses a simple INI format.

Here are the available settings:

```ini
[slideshow]
duration = 15
fps = 30
palette = vaporwave
```

Command-line arguments will always override the settings in the configuration file.

### Examples

*   Run the slideshow with a 10-second duration for each slide:
    ```bash
    ./ascii-art-show -d 10
    ```
*   Start with the "cube" module:
    ```bash
    ./ascii-art-show -s cube
    ```
*   Display a specific image:
    ```bash
    ./ascii-art-show -s image -i /path/to/your/image.png
    ```

## Interactive Controls

While the slideshow is running, you can use the following keys:

*   `p`: Pause/resume the animation.
*   `n` or `→`: Go to the next art module.
*   `b` or `←`: Go to the previous art module.
*   `i`: Toggle the information HUD.
*   `q`: Quit the application.

## Adding New Art Modules

To add a new art module, you need to:

1.  Create a new `.c` file (e.g., `art_new.c`).
2.  Implement the `ArtModule` interface (see `art.h`).
3.  Add the module to the `Makefile`.
4.  Add the module to the `art_modules` array in `main.c`.
