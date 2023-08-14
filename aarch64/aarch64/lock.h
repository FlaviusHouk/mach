#ifndef _AARCH64_LOCK_H_
#define _AARCH64_LOCK_H_

/* Basic locks for data manipulation */

/*
 * It cannot be a function with the current approach.
 * i386 defines a macro that operates on structure
 * defined in portable code and portable code includes
 * machine dependent header before simple spin lock definition.
 * Find a workaround for this later.
 */
void simple_lock_init(void* data);

#endif //_AARCH64_LOCK_H_
