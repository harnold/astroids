#include "keyb.h"
#include "bitvect.h"
#include "compat.h"
#include "dpmi.h"
#include "error.h"
#include "ringbuf.h"
#include "xmemcpy.h"

#include <conio.h>
#include <dos.h>
#include <i86.h>
#include <stdint.h>
#include <string.h>

#define KEYBOARD_INT            0x09

#define KEY_BUFFER              0x60
#define KEY_CONTROL             0x61

#define PIC_MASTER_COMMAND      0x20
#define PIC_MASTER_STATUS       0x20
#define PIC_MASTER_DATA         0x21
#define PIC_EOI                 0x20

#define MIN_SCAN_CODE           1
#define MAX_SCAN_CODE           88
#define NUM_KEYS                128

#define INPUT_BUFFER_SIZE       32

typedef void __interrupt (*key_handler_t)(void);

static int ascii_table_us[] = {
    -1,         /* KEY_NONE = 0, */
    '\e',       /* KEY_ESC = 1, */
    '1',        /* KEY_1 = 2, */
    '2',        /* KEY_2 = 3, */
    '3',        /* KEY_3 = 4, */
    '4',        /* KEY_4 = 5, */
    '5',        /* KEY_5 = 6, */
    '6',        /* KEY_6 = 7, */
    '7',        /* KEY_7 = 8, */
    '8',        /* KEY_8 = 9, */
    '9',        /* KEY_9 = 10, */
    '0',        /* KEY_0 = 11, */
    '-',        /* KEY_MINUS = 12, */
    '=',        /* KEY_EQUALS = 13, */
    '\b',       /* KEY_BACKSPACE = 14, */
    '\t',       /* KEY_TAB = 15, */
    'q',        /* KEY_Q = 16, */
    'w',        /* KEY_W = 17, */
    'e',        /* KEY_E = 18, */
    'r',        /* KEY_R = 19, */
    't',        /* KEY_T = 20, */
    'y',        /* KEY_Y = 21, */
    'u',        /* KEY_U = 22, */
    'i',        /* KEY_I = 23, */
    'o',        /* KEY_O = 24, */
    'p',        /* KEY_P = 25, */
    '[',        /* KEY_LBRACKET = 26, */
    ']',        /* KEY_RBRACKET = 27, */
    '\r',       /* KEY_ENTER = 28, */
    -1,         /* KEY_CTRL = 29, */
    'a',        /* KEY_A = 30, */
    's',        /* KEY_S = 31, */
    'd',        /* KEY_D = 32, */
    'f',        /* KEY_F = 33, */
    'g',        /* KEY_G = 34, */
    'h',        /* KEY_H = 35, */
    'j',        /* KEY_J = 36, */
    'k',        /* KEY_K = 37, */
    'l',        /* KEY_L = 38, */
    ';',        /* KEY_SEMICOLON = 39, */
    '\'',       /* KEY_APOSTROPHE = 40, */
    '`',        /* KEY_BACKQUOTE = 41, */
    -1,         /* KEY_LSHIFT = 42, */
    '\\',       /* KEY_BACKSLASH = 43, */
    'z',        /* KEY_Z = 44, */
    'x',        /* KEY_X = 45, */
    'c',        /* KEY_C = 46, */
    'v',        /* KEY_V = 47, */
    'b',        /* KEY_B = 48, */
    'n',        /* KEY_N = 49, */
    'm',        /* KEY_M = 50, */
    ',',        /* KEY_COMMA = 51, */
    '.',        /* KEY_DOT = 52, */
    '/',        /* KEY_SLASH = 53, */
    -1,         /* KEY_RSHIFT = 54, */
    -1,         /* KEY_PRINTSCR = 55, */
    -1,         /* KEY_ALT = 56, */
    ' ',        /* KEY_SPACE = 57, */
    -1,         /* KEY_CAPSLOCK = 58, */
    -1,         /* KEY_F1 = 59, */
    -1,         /* KEY_F2 = 60, */
    -1,         /* KEY_F3 = 61, */
    -1,         /* KEY_F4 = 62, */
    -1,         /* KEY_F5 = 63, */
    -1,         /* KEY_F6 = 64, */
    -1,         /* KEY_F7 = 65, */
    -1,         /* KEY_F8 = 66, */
    -1,         /* KEY_F9 = 67, */
    -1,         /* KEY_F10 = 68, */
    -1,         /* KEY_NUMLOCK = 69, */
    -1,         /* KEY_SCROLLLOCK = 70, */
    -1,         /* KEY_HOME = 71, */
    -1,         /* KEY_UP = 72, */
    -1,         /* KEY_PGUP = 73, */
    -1,         /* KEY_LEFT = 75, */
    -1,         /* KEY_RIGHT = 77, */
    -1,         /* KEY_END = 79, */
    -1,         /* KEY_DOWN = 80, */
    -1,         /* KEY_PGDOWN = 81, */
    -1,         /* KEY_INSERT = 82, */
    -1,         /* KEY_DELETE = 83, */
    '7',        /* KEY_KP_7 = 71, */
    '8',        /* KEY_KP_8 = 72, */
    '9',        /* KEY_KP_9 = 73, */
    '-',        /* KEY_KP_MINUS = 74, */
    '4',        /* KEY_KP_4 = 75, */
    '5',        /* KEY_KP_5 = 76, */
    '6',        /* KEY_KP_6 = 77, */
    '+',        /* KEY_KP_PLUS = 78, */
    '1',        /* KEY_KP_1 = 79, */
    '2',        /* KEY_KP_2 = 80, */
    '3',        /* KEY_KP_3 = 81, */
    '0',        /* KEY_KP_0 = 82, */
    -1,         /* KEY_KP_DELETE = 83 */
    -1,         /* 84 */
    -1,         /* 85 */
    -1,         /* 86 */
    -1,         /* KEY_F11 = 87, */
    -1,         /* KEY_F12 = 88, */
};

static int ascii_table_us_shift[] = {
    -1,         /* KEY_NONE = 0, */
    '\e',       /* KEY_ESC = 1, */
    '!',        /* KEY_1 = 2, */
    '@',        /* KEY_2 = 3, */
    '#',        /* KEY_3 = 4, */
    '$',        /* KEY_4 = 5, */
    '%',        /* KEY_5 = 6, */
    '^',        /* KEY_6 = 7, */
    '&',        /* KEY_7 = 8, */
    '*',        /* KEY_8 = 9, */
    '(',        /* KEY_9 = 10, */
    ')',        /* KEY_0 = 11, */
    '_',        /* KEY_MINUS = 12, */
    '+',        /* KEY_EQUALS = 13, */
    '\b',       /* KEY_BACKSPACE = 14, */
    '\t',       /* KEY_TAB = 15, */
    'Q',        /* KEY_Q = 16, */
    'W',        /* KEY_W = 17, */
    'E',        /* KEY_E = 18, */
    'R',        /* KEY_R = 19, */
    'T',        /* KEY_T = 20, */
    'Y',        /* KEY_Y = 21, */
    'U',        /* KEY_U = 22, */
    'I',        /* KEY_I = 23, */
    'O',        /* KEY_O = 24, */
    'P',        /* KEY_P = 25, */
    '{',        /* KEY_LBRACKET = 26, */
    '}',        /* KEY_RBRACKET = 27, */
    '\r',       /* KEY_ENTER = 28, */
    -1,         /* KEY_CTRL = 29, */
    'A',        /* KEY_A = 30, */
    'S',        /* KEY_S = 31, */
    'D',        /* KEY_D = 32, */
    'F',        /* KEY_F = 33, */
    'G',        /* KEY_G = 34, */
    'H',        /* KEY_H = 35, */
    'J',        /* KEY_J = 36, */
    'K',        /* KEY_K = 37, */
    'L',        /* KEY_L = 38, */
    ':',        /* KEY_SEMICOLON = 39, */
    '"',        /* KEY_APOSTROPHE = 40, */
    '~',        /* KEY_BACKQUOTE = 41, */
    -1,         /* KEY_LSHIFT = 42, */
    '|',        /* KEY_BACKSLASH = 43, */
    'Z',        /* KEY_Z = 44, */
    'X',        /* KEY_X = 45, */
    'C',        /* KEY_C = 46, */
    'V',        /* KEY_V = 47, */
    'B',        /* KEY_B = 48, */
    'N',        /* KEY_N = 49, */
    'M',        /* KEY_M = 50, */
    '<',        /* KEY_COMMA = 51, */
    '>',        /* KEY_DOT = 52, */
    '?',        /* KEY_SLASH = 53, */
    -1,         /* KEY_RSHIFT = 54, */
    -1,         /* KEY_PRINTSCR = 55, */
    -1,         /* KEY_ALT = 56, */
    ' ',        /* KEY_SPACE = 57, */
    -1,         /* KEY_CAPSLOCK = 58, */
    -1,         /* KEY_F1 = 59, */
    -1,         /* KEY_F2 = 60, */
    -1,         /* KEY_F3 = 61, */
    -1,         /* KEY_F4 = 62, */
    -1,         /* KEY_F5 = 63, */
    -1,         /* KEY_F6 = 64, */
    -1,         /* KEY_F7 = 65, */
    -1,         /* KEY_F8 = 66, */
    -1,         /* KEY_F9 = 67, */
    -1,         /* KEY_F10 = 68, */
    -1,         /* KEY_NUMLOCK = 69, */
    -1,         /* KEY_SCROLLLOCK = 70, */
    -1,         /* KEY_HOME = 71, */
    -1,         /* KEY_UP = 72, */
    -1,         /* KEY_PGUP = 73, */
    -1,         /* KEY_LEFT = 75, */
    -1,         /* KEY_RIGHT = 77, */
    -1,         /* KEY_END = 79, */
    -1,         /* KEY_DOWN = 80, */
    -1,         /* KEY_PGDOWN = 81, */
    -1,         /* KEY_INSERT = 82, */
    -1,         /* KEY_DELETE = 83, */
    -1,         /* KEY_KP_7 = 71, */
    -1,         /* KEY_KP_8 = 72, */
    -1,         /* KEY_KP_9 = 73, */
    -1,         /* KEY_KP_MINUS = 74, */
    -1,         /* KEY_KP_4 = 75, */
    -1,         /* KEY_KP_5 = 76, */
    -1,         /* KEY_KP_6 = 77, */
    -1,         /* KEY_KP_PLUS = 78, */
    -1,         /* KEY_KP_1 = 79, */
    -1,         /* KEY_KP_2 = 80, */
    -1,         /* KEY_KP_3 = 81, */
    -1,         /* KEY_KP_0 = 82, */
    -1,         /* KEY_KP_DELETE = 83 */
    -1,         /* 84 */
    -1,         /* 85 */
    -1,         /* 86 */
    -1,         /* KEY_F11 = 87, */
    -1,         /* KEY_F12 = 88, */
};

static bool keyboard_initialized;

static struct {
    key_handler_t default_handler;
    volatile uint8_t key_vector[(NUM_KEYS + 7) / 8];
    volatile unsigned modifiers;
    volatile struct ringbuf input_buffer;
    volatile uint8_t input_buffer_data[INPUT_BUFFER_SIZE];
} keyboard;

static inline int scancode_us_to_ascii(unsigned key, unsigned modifiers)
{
    if (key > MAX_SCAN_CODE)
        return -1;

    return (modifiers & MOD_SHIFT) ?
        ascii_table_us_shift[key] :
        ascii_table_us[key];
}

#pragma off (check_stack)

static void __interrupt key_handler(union INTPACK regs)
{
    unsigned key_code;
    unsigned key_control;

    UNUSED(regs);

    key_code = inp(KEY_BUFFER);
    key_control = inp(KEY_CONTROL);
    outp(KEY_CONTROL, key_control | 0x80u);
    outp(KEY_CONTROL, key_control & ~0x80u);

    if (key_code >= MIN_SCAN_CODE &&
        key_code <= MAX_SCAN_CODE) {

        vbitvect_set_bit(keyboard.key_vector, key_code);

        if (key_code == KEY_LSHIFT || key_code == KEY_RSHIFT)
            keyboard.modifiers |= MOD_SHIFT;
        else if (key_code == KEY_CTRL)
            keyboard.modifiers |= MOD_CTRL;
        else if (key_code == KEY_ALT)
            keyboard.modifiers |= MOD_ALT;

        int ch = scancode_us_to_ascii(key_code, keyboard.modifiers);

        if (ch > 0) {
            unsigned char c = (unsigned char) ch;
            ringbuf_push_back((struct ringbuf *) &keyboard.input_buffer,
                              (unsigned char *) &c, 1);
        }

    } else if (key_code >= (MIN_SCAN_CODE | 0x80u) &&
               key_code <= (MAX_SCAN_CODE | 0x80u)) {

        vbitvect_clear_bit(keyboard.key_vector, key_code & ~0x80u);

        if (key_code == KEY_LSHIFT || key_code == KEY_RSHIFT)
            keyboard.modifiers &= ~MOD_SHIFT;
        else if (key_code == KEY_CTRL)
            keyboard.modifiers &= ~MOD_CTRL;
        else if (key_code == KEY_ALT)
            keyboard.modifiers &= ~MOD_ALT;
    }

    outp(PIC_MASTER_COMMAND, PIC_EOI);
}

static void key_handler_end(void) {}

#pragma on (check_stack)

int keyboard_init(void)
{
    if (keyboard_initialized)
        return 0;

    if (dpmi_lock_linear_region((uint32_t) &keyboard, sizeof(keyboard)) != 0)
        return error("Locking keyboard data failed");

    if (dpmi_lock_linear_region(
            (uint32_t) key_handler,
            (char *) key_handler_end - (char *) key_handler) != 0) {
        dpmi_unlock_linear_region((uint32_t) &keyboard, sizeof(keyboard));
        return error("Locking keyboard interrupt handler failed");
    }

    xmemset(&keyboard, 0, sizeof(keyboard));
    init_ringbuf((struct ringbuf *) &keyboard.input_buffer,
                 (void *) keyboard.input_buffer_data,
                 sizeof(keyboard.input_buffer_data));

    _disable();
    keyboard.default_handler = _dos_getvect(KEYBOARD_INT);
    _dos_setvect(KEYBOARD_INT, key_handler);
    _enable();

    keyboard_initialized = true;
    return 0;
}

void keyboard_exit(void)
{
    if (!keyboard_initialized)
        return;

    _disable();
    _dos_setvect(KEYBOARD_INT, keyboard.default_handler);
    _enable();
}

bool key_pressed(uint8_t key_code)
{
    return vbitvect_test_bit(keyboard.key_vector, key_code);
}

bool any_key_pressed(const uint8_t *key_codes, size_t n)
{
    for (size_t i = 0; i < n; i++, key_codes++) {
        if (key_pressed(*key_codes))
            return true;
    }

    return false;
}

unsigned key_modifiers(void)
{
    return keyboard.modifiers;
}

int keyboard_get_char(void)
{
    unsigned char ch;

    _disable();
    int res = ringbuf_pop_front((struct ringbuf *) &keyboard.input_buffer, &ch, 1);
    _enable();

    return res == 0 ? ch : -1;
}

int keyboard_get_char_wait(void)
{
    unsigned char ch;

    while (ringbuf_empty((struct ringbuf *) &keyboard.input_buffer)) {
        /* Wait. */
    }

    _disable();
    int res = ringbuf_pop_front((struct ringbuf *) &keyboard.input_buffer, &ch, 1);
    _enable();

    return res == 0 ? ch : -1;
}

void keyboard_clear_input_buffer(void)
{
    _disable();
    ringbuf_clear((struct ringbuf *) &keyboard.input_buffer);
    _enable();
}
