#ifndef _MACHINE_AARCH64_VM_PARAM_H_
#define _MACHINE_AARCH64_VM_PARAM_H_

/* Parameters of the virtual memory  */

/* 
 * AArch64 can work with 4K or 64K page sizes.
 * Probably one defines the size here. However
 * I do not know how to properly work with these values yet. 
 */


#define PAGE_SHIFT 12
#define TABLE_SHIFT 9
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)

//4K
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define SECTION_SIZE (1 << SECTION_SHIFT)

#define LOW_MEMORY (2 * SECTION_SIZE)

#ifndef __ASSEMBLER__

void memzero(unsigned long src, unsigned int n);

#endif

#endif //_MACHINE_AARCH64_VM_PARAM_H_
