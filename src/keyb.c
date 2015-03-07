#include "keyb.h"
#include "bitvect.h"
#include "compat.h"
#include "dpmi.h"
#include "error.h"
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

typedef void __interrupt (*key_handler_t)();

static bool keyboard_initialized;

static struct {
    key_handler_t default_handler;
    volatile uint8_t key_vector[(NUM_KEYS + 7) / 8];
    volatile unsigned modifiers;
} keyboard;

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

    } else if (key_code >= (MIN_SCAN_CODE | 0x80) &&
               key_code <= (MAX_SCAN_CODE | 0x80)) {

        vbitvect_clear_bit(keyboard.key_vector, key_code & ~0x80);

        if (key_code == KEY_LSHIFT || key_code == KEY_RSHIFT)
            keyboard.modifiers &= ~MOD_SHIFT;
        else if (key_code == KEY_CTRL)
            keyboard.modifiers &= ~MOD_CTRL;
        else if (key_code == KEY_ALT)
            keyboard.modifiers &= ~MOD_ALT;
    }
}

static void key_handler_end(void) {}

#pragma on (check_stack)

int keyboard_init(void)
{
    if (dpmi_lock_linear_region((uint32_t) &keyboard, sizeof(keyboard)) != 0)
        return error("Locking keyboard data failed");

    if (dpmi_lock_linear_region(
            (uint32_t) key_handler,
            (char *) key_handler_end - (char *) key_handler) != 0) {
        dpmi_unlock_linear_region((uint32_t) &keyboard, sizeof(keyboard));
        return error("Locking keyboard interrupt handler failed");
    }

    keyboard.default_handler = _dos_getvect(KEYBOARD_INT);
    xmemset((void *) keyboard.key_vector, 0, sizeof(keyboard.key_vector));

    _disable();
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

bool key_pressed(unsigned key_code)
{
    return vbitvect_test_bit(keyboard.key_vector, key_code);
}

unsigned key_modifiers(void)
{
    return keyboard.modifiers;
}
