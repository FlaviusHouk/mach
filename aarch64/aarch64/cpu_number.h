#ifndef	_AARCH64_CPU_NUMBER_H_
#define	_AARCH64_CPU_NUMBER_H_

/* 
 * Written for Raspberry Pi 4 (AArch64, Armv8 Cortex A72, BCM2711)
 * For A72 processors it is possible to have up to 4 cores.
 * It might be obtained from the MPIDR_EL1 (available on Execution 
 * level 1 and higher) from the lower bits (0:1)
 */

#define	CPU_NUMBER(reg)	\
	mrs     reg, MPIDR_EL1		;\
    and     reg, reg, 3;


#ifndef __ASSEMBLER__
#include "kern/cpu_number.h"
int cpu_number(void);
#endif

#endif	/* _AARCH64_CPU_NUMBER_H_ */
