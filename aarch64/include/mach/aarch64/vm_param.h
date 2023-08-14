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

//21
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)

//4K
#define PAGE_SIZE (1 << PAGE_SHIFT)

//2M 0x200000
//Isn't it a size of the single entry in L2 table translation?
#define SECTION_SIZE (1 << SECTION_SHIFT)

//4M 0x400000
//Number of entries mapped to some important addresses?
#define LOW_MEMORY (2 * SECTION_SIZE)
#define VM_MIN_KERNEL_ADDRESS LOW_MEMORY

#ifndef __ASSEMBLER__

void memzero(unsigned long src, unsigned int n);

#endif

#endif //_MACHINE_AARCH64_VM_PARAM_H_
