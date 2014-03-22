#include "timer.h"
#include "dpmi.h"
#include "error.h"

#include <conio.h>
#include <dos.h>
#include <stdbool.h>

#define PIT_SYSTEM_INT      0x08
#define PIT_USER_INT        0x1C

#define PIT_BASE_FREQUENCY  (3579545.0 / 3)
#define PIT_COUNTER_MAX     65536

#define PIT_CH0_DATA        0x40
#define PIT_COMMAND         0x43

#define PIC_MASTER_COMMAND  0x20
#define PIC_MASTER_DATA     0x21
#define PIC_EOI             0x20

typedef void __interrupt (*timer_handler_t)();

static bool timer_initialized;

static struct {
    timer_handler_t default_handler;
    int counter_init;
    float ticks_per_sec;
    volatile unsigned int ticks;
    unsigned int last_ticks_value;
} timer;

#pragma off (check_stack)

static void __interrupt timer_handler(union INTPACK regs)
{
    static int bios_counter = 0;

    timer.ticks++;
    bios_counter += timer.counter_init;

    if (bios_counter >= PIT_COUNTER_MAX) {
        bios_counter -= PIT_COUNTER_MAX;
        _chain_intr(timer.default_handler);
    } else {
        outp(PIC_MASTER_COMMAND, PIC_EOI);
    }
}

static void timer_handler_end(void) {}

#pragma on (check_stack)

int timer_init(float ticks_per_sec)
{
    timer.counter_init = (int) (PIT_BASE_FREQUENCY / ticks_per_sec);

    if (timer.counter_init >= PIT_COUNTER_MAX) {
        return error("Requested timer frequency of %.2f Hz too low;"
                     "minimum frequency is 18.2 Hz", ticks_per_sec);
    } else if (timer.counter_init < 1) {
        return error("Requested timer frequency of %.2f Hz too high;"
                     "maximum frequency is 1.193181 MHz", ticks_per_sec);
    }

    timer.ticks_per_sec = PIT_BASE_FREQUENCY / timer.counter_init;
    timer.ticks = 0;
    timer.last_ticks_value = 0;

    if (dpmi_lock_linear_region((uint32_t) &timer, sizeof(timer)) != 0)
        return error("Locking timer data failed");

    if (dpmi_lock_linear_region(
            (uint32_t) timer_handler,
            (char *) timer_handler_end - (char *) timer_handler) != 0) {
        dpmi_unlock_linear_region((uint32_t) &timer, sizeof(timer));
        return error("Locking timer interrupt handler failed");
    }

    timer.default_handler = _dos_getvect(PIT_SYSTEM_INT);

    _disable();

    /* Set channel 0 to rate generator mode, <ticks_per_sec> Hz. */

    outp(PIT_COMMAND, 52);
    outp(PIT_CH0_DATA, timer.counter_init & 0xFF);
    outp(PIT_CH0_DATA, timer.counter_init >> 8);

    _dos_setvect(PIT_SYSTEM_INT, timer_handler);

    _enable();

    timer_initialized = true;
    return 0;
}

void timer_exit(void)
{
    if (!timer_initialized)
        return;

    _disable();

    /* Reset channel 0 to square wave mode, 18.2 Hz. */

    outp(PIT_COMMAND, 54);
    outp(PIT_CH0_DATA, 0x00);
    outp(PIT_CH0_DATA, 0x00);

    _dos_setvect(PIT_SYSTEM_INT, timer.default_handler);

    _enable();

    if (dpmi_unlock_linear_region((uint32_t) &timer, sizeof(timer)) != 0)
        error("Unlocking timer data failed");

    if (dpmi_unlock_linear_region(
            (uint32_t) &timer_handler,
            (char *) timer_handler_end - (char *) timer_handler) != 0)
        error(" Unlocking timer interrupt handler failed");
}

float timer_ticks_per_sec(void)
{
    return timer.ticks_per_sec;
}

unsigned int timer_get_ticks(void)
{
    timer.last_ticks_value = timer.ticks;
    return timer.last_ticks_value;
}

unsigned int timer_get_ticks_delta(void)
{
    unsigned int delta = timer.ticks - timer.last_ticks_value;
    timer.last_ticks_value += delta;
    return delta;
}

float timer_get_time(void)
{
    timer.last_ticks_value = timer.ticks;
    return timer.last_ticks_value / timer.ticks_per_sec;
}

float timer_get_time_delta(void)
{
    unsigned int delta = timer.ticks - timer.last_ticks_value;
    timer.last_ticks_value += delta;
    return delta / timer.ticks_per_sec;
}

void timer_sleep(float seconds)
{
    unsigned int end_of_sleep = timer.ticks + seconds * timer.ticks_per_sec;
    while (timer.ticks < end_of_sleep) {
        /* do nothing */
    }
}

void timer_wait(float time)
{
    unsigned int end_of_sleep = time * timer.ticks_per_sec;
    while (timer.ticks < end_of_sleep) {
        /* do nothing */
    }
}
