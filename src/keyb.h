#ifndef KEYB_H
#define KEYB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum key_code {
    KEY_NONE = 0,
    KEY_ESC = 1,
    KEY_1 = 2,
    KEY_2 = 3,
    KEY_3 = 4,
    KEY_4 = 5,
    KEY_5 = 6,
    KEY_6 = 7,
    KEY_7 = 8,
    KEY_8 = 9,
    KEY_9 = 10,
    KEY_0 = 11,
    KEY_MINUS = 12,
    KEY_EQUALS = 13,
    KEY_BACKSPACE = 14,
    KEY_TAB = 15,
    KEY_Q = 16,
    KEY_W = 17,
    KEY_E = 18,
    KEY_R = 19,
    KEY_T = 20,
    KEY_Y = 21,
    KEY_U = 22,
    KEY_I = 23,
    KEY_O = 24,
    KEY_P = 25,
    KEY_LBRACKET = 26,
    KEY_RBRACKET = 27,
    KEY_ENTER = 28,
    KEY_CTRL = 29,
    KEY_A = 30,
    KEY_S = 31,
    KEY_D = 32,
    KEY_F = 33,
    KEY_G = 34,
    KEY_H = 35,
    KEY_J = 36,
    KEY_K = 37,
    KEY_L = 38,
    KEY_SEMICOLON = 39,
    KEY_APOSTROPHE = 40,
    KEY_BACKQUOTE = 41,
    KEY_LSHIFT = 42,
    KEY_BACKSLASH = 43,
    KEY_Z = 44,
    KEY_X = 45,
    KEY_C = 46,
    KEY_V = 47,
    KEY_B = 48,
    KEY_N = 49,
    KEY_M = 50,
    KEY_COMMA = 51,
    KEY_DOT = 52,
    KEY_SLASH = 53,
    KEY_RSHIFT = 54,
    KEY_PRINTSCR = 55,
    KEY_ALT = 56,
    KEY_SPACE = 57,
    KEY_CAPSLOCK = 58,
    KEY_F1 = 59,
    KEY_F2 = 60,
    KEY_F3 = 61,
    KEY_F4 = 62,
    KEY_F5 = 63,
    KEY_F6 = 64,
    KEY_F7 = 65,
    KEY_F8 = 66,
    KEY_F9 = 67,
    KEY_F10 = 68,
    KEY_NUMLOCK = 69,
    KEY_SCROLLLOCK = 70,
    KEY_HOME = 71,
    KEY_UP = 72,
    KEY_PGUP = 73,
    KEY_LEFT = 75,
    KEY_RIGHT = 77,
    KEY_END = 79,
    KEY_DOWN = 80,
    KEY_PGDOWN = 81,
    KEY_INSERT = 82,
    KEY_DELETE = 83,
    KEY_KP_7 = 71,
    KEY_KP_8 = 72,
    KEY_KP_9 = 73,
    KEY_KP_MINUS = 74,
    KEY_KP_4 = 75,
    KEY_KP_5 = 76,
    KEY_KP_6 = 77,
    KEY_KP_PLUS = 78,
    KEY_KP_1 = 79,
    KEY_KP_2 = 80,
    KEY_KP_3 = 81,
    KEY_KP_0 = 82,
    KEY_KP_DELETE = 83,
    KEY_F11 = 87,
    KEY_F12 = 88,
};

enum modifier_keys {
    MOD_SHIFT = 1,
    MOD_CTRL = 2,
    MOD_ALT = 4
};

int keyboard_init(void);
void keyboard_exit(void);
bool key_pressed(uint8_t key_code);
bool any_key_pressed(const uint8_t *key_codes, size_t n);
unsigned key_modifiers(void);
int keyboard_get_char(void);
int keyboard_get_char_wait(void);
void keyboard_clear_input_buffer(void);

#endif
