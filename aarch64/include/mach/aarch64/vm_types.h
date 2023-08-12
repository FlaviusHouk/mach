#ifndef	_MACHINE_AARCH64_VM_TYPES_H_
#define _MACHINE_AARCH64_VM_TYPES_H_	1

#ifndef __ASSEMBLER__

#include <stdint.h>

#ifdef MACH_KERNEL
#include <kern/assert.h>
#endif

/*
 * A natural_t is the type for the native
 * unsigned integer type, usually 32 bits. It is suitable for
 * most counters with a small chance of overflow.
 * While historically natural_t was meant to be the same
 * as a pointer, that is not the case here.
 */
typedef unsigned int	natural_t;

/*
 * An integer_t is the signed counterpart
 * of the natural_t type. Both types are
 * only supposed to be used to define
 * other types in a machine-independent
 * way.
 */
typedef int		integer_t;

/*
 * A long_natural_t is a possibly larger unsigned integer type than natural_t.
 * Should be used instead of natural_t when we want the data to be less subject
 * to overflows.
 */
typedef unsigned long long long_natural_t;

/*
 * Larger version of integer_t. Only used when we want to hold possibly larger
 * values than what is possible with integer_t.
 */
typedef long long long_integer_t;

/*
 * A vm_offset_t is a type-neutral pointer,
 * e.g. an offset into a virtual memory space.
 */
typedef	uintptr_t	vm_offset_t;
typedef	vm_offset_t *	vm_offset_array_t;

/*
 * A type for physical addresses.
 */
typedef unsigned long long phys_addr_t;
typedef unsigned long long rpc_phys_addr_t;

/*
 * A vm_size_t is the proper type for e.g.
 * expressing the difference between two
 * vm_offset_t entities.
 */
typedef uintptr_t vm_size_t;
typedef	vm_size_t *	vm_size_array_t;

/*
 * rpc_types are for user/kernel interfaces. On kernel side they may differ from
 * the native types, while on user space they shall be the same.
 * These three types are always of the same size, so we can reuse the conversion
 * functions.
 */
#if defined(MACH_KERNEL)
typedef uint32_t	rpc_uintptr_t;
typedef uint32_t	rpc_vm_address_t;
typedef uint32_t	rpc_vm_offset_t;
typedef uint32_t	rpc_vm_size_t;

static inline uint64_t convert_vm_from_user(uint32_t uaddr)
{
    return (uint64_t)uaddr;
}

static inline uint32_t convert_vm_to_user(uint64_t kaddr)
{
    assert(kaddr <= 0xFFFFFFFF);
    return (uint32_t)kaddr;
}

typedef uint32_t rpc_long_natural_t;
typedef int32_t rpc_long_integer_t;

static inline int64_t convert_long_integer_from_user(int32_t i)
{
	return (int64_t)i;
}
static inline int32_t convert_long_integer_to_user(int64_t i)
{
    assert(i <= 0x7FFFFFFF);
	return (int32_t)i;
}
#else /* MACH_KERNEL */
typedef uintptr_t	rpc_uintptr_t;
typedef vm_offset_t	rpc_vm_address_t;
typedef vm_offset_t	rpc_vm_offset_t;
typedef vm_size_t	rpc_vm_size_t;

#define convert_vm_to_user null_conversion
#define convert_vm_from_user null_conversion

typedef long_natural_t rpc_long_natural_t;
typedef long_integer_t rpc_long_integer_t;

#define convert_long_integer_to_user null_conversion
#define convert_long_integer_from_user null_conversion
#endif /* MACH_KERNEL */

#define convert_long_natural_to_user convert_vm_to_user
#define convert_long_natural_from_user convert_vm_from_user

typedef	rpc_vm_size_t *	rpc_vm_size_array_t;
typedef	rpc_vm_offset_t *	rpc_vm_offset_array_t;

#endif	/* __ASSEMBLER__ */

/*
 * If composing messages by hand (please dont)
 */

#define	MACH_MSG_TYPE_INTEGER_T	MACH_MSG_TYPE_INTEGER_32

#endif	/* _MACHINE_VM_TYPES_H_ */
