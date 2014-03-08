#include "dpmi.h"
#include "error.h"

#include <i86.h>
#include <string.h>

#define DPMI_INT        0x31

struct dpmi_error {
    unsigned int code;
    const char *message;
};

static const struct dpmi_error dpmi_error_table[] = {
    0x8001, "Function not supported",
    0x8002, "Invalid state",
    0x8003, "System integrity",
    0x8004, "Deadlock detected",
    0x8005, "Request cancelled",
    0x8010, "Resource unavailable",
    0x8011, "Descriptor unavailable",
    0x8012, "Linear memory unavailable",
    0x8013, "Physical memory unavailable",
    0x8014, "Backing store unavailable",
    0x8015, "Callback unavailable",
    0x8016, "Handle unavailable",
    0x8017, "Lock count exceeded",
    0x8018, "Resource owned exclusively",
    0x8019, "Resource owned shared",
    0x8021, "Invalid value",
    0x8022, "Invalid selector",
    0x8023, "Invalid handle",
    0x8024, "Invalid callback",
    0x8025, "Invalid linear address",
    0x8026, "Invalid request"
};

static const char *dpmi_error_message(int error_code)
{
    const struct dpmi_error *e = &dpmi_error_table;
    const struct dpmi_error *end = e + array_length(dpmi_error_table);

    while (e < end) {
        if (error_code == e->code)
            return e->message;
        e++;
    }

    return "Unknown error";
}

static int dpmi_error(int fn, int error_code)
{
    if (error_code < 0x8000) {
        return error("DPMI function %04Xh failed: DOS error code %d",
                     fn, error_code);
    } else {
        return error("DPMI function %04Xh failed: %s",
                     fn, dpmi_error_message(error_code));
    }
}

static inline int dpmi_call_function(int fn, union REGS *regs)
{
    regs->w.ax = (uint16_t) fn;
    int386(DPMI_INT, regs, regs);

    if (regs->x.cflag != 0)
        return dpmi_error(fn, regs->w.ax);

    return 0;
}

int dpmi_map_physical_address(uint32_t phys_addr, uint32_t size, uint32_t *lin_addr)
{
    union REGS regs;

    memset(&regs, 0, sizeof(regs));
    regs.w.bx = hword(phys_addr);
    regs.w.cx = lword(phys_addr);
    regs.w.si = hword(size);
    regs.w.di = lword(size);

    if (dpmi_call_function(0x0800, &regs) != 0)
        return -1;

    *lin_addr = ((uint32_t) regs.w.bx << 16 ) | regs.w.cx;
    return 0;
}

int dpmi_unmap_physical_address(uint32_t lin_addr)
{
    union REGS regs;

    memset(&regs, 0, sizeof(regs));
    regs.w.bx = hword(lin_addr);
    regs.w.cx = lword(lin_addr);

    return dpmi_call_function(0x0801, &regs);
}

int dpmi_allocate_dos_memory(uint32_t size, uint16_t *segment, uint16_t *selector)
{
    union REGS regs;

    if (size > 0xFFFF0) {
        return error("Cannot allocate DOS memory block larger than %u bytes "
                     "(%u bytes requested)", 0xFFFF0, size);
    }

    memset(&regs, 0, sizeof(regs));
    regs.w.bx = (uint16_t) ((size + 15) >> 4);

    if (dpmi_call_function(0x0100, &regs) != 0)
        return -1;

    *segment  = regs.w.ax;
    *selector = regs.w.dx;
    return 0;
}

int dpmi_free_dos_memory(uint16_t selector)
{
    union REGS regs;

    memset(&regs, 0, sizeof(regs));
    regs.w.dx = selector;

    return dpmi_call_function(0x0101, &regs);
}

int dpmi_simulate_rm_interrupt(int inum, struct dpmi_rm_info *info)
{
    union REGS regs;
    struct SREGS sregs;

    memset(&regs, 0, sizeof(regs));
    memset(&sregs, 0, sizeof(sregs));
    regs.h.bl = (uint8_t) inum;
    sregs.es = FP_SEG(info);
    regs.x.edi = FP_OFF(info);

    return dpmi_call_function(0x0300, &regs);
}

int dpmi_lock_linear_region(uint32_t lin_addr, uint32_t size)
{
    union REGS regs;

    memset(&regs, 0, sizeof(regs));
    regs.w.bx = hword(lin_addr);
    regs.w.cx = lword(lin_addr);
    regs.w.si = hword(size);
    regs.w.di = lword(size);

    return dpmi_call_function(0x0600, &regs);
}

int dpmi_unlock_linear_region(uint32_t lin_addr, uint32_t size)
{
    union REGS regs;

    memset(&regs, 0, sizeof(regs));
    regs.w.bx = hword(lin_addr);
    regs.w.cx = lword(lin_addr);
    regs.w.si = hword(size);
    regs.w.di = lword(size);

    return dpmi_call_function(0x0601, &regs);
}
