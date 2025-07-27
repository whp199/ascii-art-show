#include "terminal.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>

static struct termios orig_termios;
static volatile sig_atomic_t resized_flag = 1;

static void handle_winch(int sig) {
    (void)sig;
    resized_flag = 1;
}

void setup_terminal() {
    // Save original terminal settings
    tcgetattr(STDIN_FILENO, &orig_termios);

    // Set up signal handler for window resize
    signal(SIGWINCH, handle_winch);

    struct termios raw = orig_termios;
    // Set to non-canonical mode (process input char-by-char)
    // and disable echo
    raw.c_lflag &= ~(ICANON | ECHO);
    // Set non-blocking input
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    // Hide cursor
    printf("\e[?25l");
    fflush(stdout);
}

void cleanup_terminal() {
    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);

    // Show cursor and clear screen
    printf("\e[?25h\e[0m\e[2J\e[H");
    fflush(stdout);
}

int term_get_width() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

int term_get_height() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
}

int term_has_resized() {
    if (resized_flag) {
        resized_flag = 0;
        return 1;
    }
    return 0;
}

int term_get_key() {
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == '\x1b') { // Escape sequence
            char seq[3];
            if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
            if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
            if (seq[0] == '[') {
                switch (seq[1]) {
                    case 'A': return KEY_UP;
                    case 'B': return KEY_DOWN;
                    case 'C': return KEY_RIGHT;
                    case 'D': return KEY_LEFT;
                }
            }
            return '\x1b';
        }
        return c;
    }
    return -1; // No key pressed
}