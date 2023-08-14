#include "aarch64/vm_params.h"

.globl _start
_start:
	adr x0, bss_begin
	addr x1, bss_end
	sub x1, x1, x0
	bl memzero

	mov sp, #LOW_MEMORY
	bl c_boot_entry

.globl memzero
memzero:
	str xzr, [x0], #8
	subs x1, x1, #8
	b.gt memzero
	ret
