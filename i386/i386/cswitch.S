/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

#include <mach/machine/asm.h>

#include <i386/proc_reg.h>
#include <i386/i386asm.h>
#include <i386/cpu_number.h>
#include <i386/gdt.h>

/*
 * Context switch routines for i386.
 */

ENTRY(Load_context)
	movl	S_ARG0,%ecx			/* get thread */
	movl	TH_KERNEL_STACK(%ecx),%ecx	/* get kernel stack */
	lea	KERNEL_STACK_SIZE-IKS_SIZE-IEL_SIZE(%ecx),%edx
						/* point to stack top */
	CPU_NUMBER(%eax)
	movl	%ecx,MY(ACTIVE_STACK)		/* store stack address */
	movl	%edx,CX(EXT(kernel_stack),%eax)	/* store stack top */

	movl	KSS_ESP(%ecx),%esp		/* switch stacks */
	movl	KSS_ESI(%ecx),%esi		/* restore registers */
	movl	KSS_EDI(%ecx),%edi
	movl	KSS_EBP(%ecx),%ebp
	movl	KSS_EBX(%ecx),%ebx
	xorl	%eax,%eax			/* return zero (no old thread) */
	jmp	*KSS_EIP(%ecx)			/* resume thread */

/*
 *	This really only has to save registers
 *	when there is no explicit continuation.
 */

ENTRY(Switch_context)
	movl	MY(ACTIVE_STACK),%ecx		/* get old kernel stack */

	movl	%ebx,KSS_EBX(%ecx)		/* save registers */
	movl	%ebp,KSS_EBP(%ecx)
	movl	%edi,KSS_EDI(%ecx)
	movl	%esi,KSS_ESI(%ecx)
	popl	KSS_EIP(%ecx)			/* save return PC */
	movl	%esp,KSS_ESP(%ecx)		/* save SP */

	movl	0(%esp),%eax			/* get old thread */
	movl	%ecx,TH_KERNEL_STACK(%eax)	/* save old stack */
	movl	4(%esp),%ebx			/* get continuation */
	movl	%ebx,TH_SWAP_FUNC(%eax)		/* save continuation */

	movl	8(%esp),%esi			/* get new thread */

	movl	TH_KERNEL_STACK(%esi),%ecx	/* get its kernel stack */
	lea	KERNEL_STACK_SIZE-IKS_SIZE-IEL_SIZE(%ecx),%ebx
						/* point to stack top */

	CPU_NUMBER(%edx)
	movl	%esi,MY(ACTIVE_THREAD)		/* new thread is active */
	movl	%ecx,MY(ACTIVE_STACK)		/* set current stack */
	movl	%ebx,CX(EXT(kernel_stack),%edx)	/* set stack top */

	movl	KSS_ESP(%ecx),%esp		/* switch stacks */
	movl	KSS_ESI(%ecx),%esi		/* restore registers */
	movl	KSS_EDI(%ecx),%edi
	movl	KSS_EBP(%ecx),%ebp
	movl	KSS_EBX(%ecx),%ebx
	jmp	*KSS_EIP(%ecx)			/* return old thread */

ENTRY(Thread_continue)
	pushl	%eax				/* push the thread argument */
	xorl	%ebp,%ebp			/* zero frame pointer */
	call	*%ebx				/* call real continuation */

#if	NCPUS > 1
/*
 * void switch_to_shutdown_context(thread_t thread,
 *				   void (*routine)(processor_t),
 *				   processor_t processor)
 *
 * saves the kernel context of the thread,
 * switches to the interrupt stack,
 * continues the thread (with thread_continue),
 * then runs routine on the interrupt stack.
 *
 * Assumes that the thread is a kernel thread (thus
 * has no FPU state)
 */
ENTRY(switch_to_shutdown_context)
	movl	MY(ACTIVE_STACK),%ecx		/* get old kernel stack */
	movl	%ebx,KSS_EBX(%ecx)		/* save registers */
	movl	%ebp,KSS_EBP(%ecx)
	movl	%edi,KSS_EDI(%ecx)
	movl	%esi,KSS_ESI(%ecx)
	popl	KSS_EIP(%ecx)			/* save return PC */
	movl	%esp,KSS_ESP(%ecx)		/* save SP */

	movl	0(%esp),%eax			/* get old thread */
	movl	%ecx,TH_KERNEL_STACK(%eax)	/* save old stack */
	movl	$0,TH_SWAP_FUNC(%eax)		/* clear continuation */
	movl	4(%esp),%ebx			/* get routine to run next */
	movl	8(%esp),%esi			/* get its argument */

	CPU_NUMBER(%edx)
	movl	CX(EXT(int_stack_base),%edx),%ecx	/* point to its interrupt stack */
	lea	-4+INTSTACK_SIZE(%ecx),%esp	/* switch to it (top) */

	pushl	%eax				/* push thread */
	call	EXT(thread_dispatch)		/* reschedule thread */
	addl	$4,%esp				/* clean stack */

	pushl	%esi				/* push argument */
	call	*%ebx				/* call routine to run */
	hlt					/* (should never return) */

#endif	/* NCPUS > 1 */
