#ifndef TERMINAL_H
#define TERMINAL_H

// Special key codes for arrow keys
#define KEY_UP 1000
#define KEY_DOWN 1001
#define KEY_RIGHT 1002
#define KEY_LEFT 1003

// Sets up the terminal for the application (non-blocking, no echo, etc.)
void setup_terminal();

// Restores the terminal to its original state
void cleanup_terminal();

// Gets the current width (columns) of the terminal
int term_get_width();

// Gets the current height (rows) of the terminal
int term_get_height();

// Checks if the terminal has been resized since the last check
int term_has_resized();

// Gets a key press without blocking. Returns -1 if no key is pressed.
int term_get_key();

#endif // TERMINAL_H