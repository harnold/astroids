#ifndef DPMI_H
#define DPMI_H

#include "compat.h"

PACKED_STRUCT dpmi_rm_info {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t reserved;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint16_t flags;
    uint16_t es, ds, fs, gs, ip, cs, sp, ss;
};

int dpmi_map_physical_address(uint32_t phys_addr, uint32_t size, uint32_t *lin_addr);
int dpmi_unmap_physical_address(uint32_t lin_addr);
int dpmi_allocate_dos_memory(uint32_t size, uint16_t *segment, uint16_t *selector);
int dpmi_free_dos_memory(uint16_t selector);
int dpmi_simulate_rm_interrupt(int inum, struct dpmi_rm_info *info);
int dpmi_lock_linear_region(uint32_t lin_addr, uint32_t size);
int dpmi_unlock_linear_region(uint32_t lin_addr, uint32_t size);

static inline void *dpmi_ptr_to_rm_segment(uint16_t rm_seg)
{
    return (void *) ((uint32_t) rm_seg << 4);
}

static inline void *dpmi_ptr_to_rm_address(uint32_t rm_addr)
{
    return (void *) (((rm_addr & 0xFFFF0000) >> 12) + (rm_addr & 0xFFFF));
}

#endif
