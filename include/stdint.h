#ifndef _STDINT_H_
#define _STDINT_H_

#include <sys/types.h>
#include <machine/limits.h>

/*
 * Limits of exact-width types.
 */
#define INT8_MAX    127
#define INT16_MAX   32767
#define INT32_MAX   2147483647
#define INT64_MAX   9223372036854775807LL

#define INT8_MIN    -128
#define INT16_MIN   -32768
#define INT32_MIN   (-INT32_MAX-1)
#define INT64_MIN   (-INT64_MAX-1)

#define UINT8_MAX   255
#define UINT16_MAX  65535
#define UINT32_MAX  4294967295U
#define UINT64_MAX  18446744073709551615ULL

#endif /* _STDINT_H_ */
