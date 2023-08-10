#ifndef _MACH_AARCH64_DEFS_H_
#define _MACH_AARCH64_DEFS_H_

/* 
 * I don't understand where do these values 
 * come from and why are they being used here
 */

#define PAGE_SHIFT 12
#define TABLE_SHIFT 9
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define SECTION_SIZE (1 << SECTION_SHIFT)

#define LOW_MEMORY (2 * SECTION_SIZE)

#ifndef __ASSEMBLER__

void memzero(unsigned long src, unsigned int n);

#endif

#endif //_MACH_AARCH64_DEFS_H_
