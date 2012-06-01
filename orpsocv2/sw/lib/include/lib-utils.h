#ifndef _LIB_UTILS_H_
#define _LIB_UTILS_H_

// null from stddef.h
#define NULL ((void *)0)
// valist stuff from stddef.h
typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;
#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)
#define va_copy(d,s)    __builtin_va_copy(d,s)

// size_t and wchar definitions
typedef unsigned int size_t;
// wchar def
#ifndef __WCHAR_TYPE__
#define __WCHAR_TYPE__ int
#endif
#ifndef __cplusplus
typedef __WCHAR_TYPE__ wchar_t;
#endif

/* memcpy */
void* memcpy( void* s1, void* s2, size_t n);

/* strlen */
size_t strlen(const char*s);  

/* memchr */
void *memchr(const void *s, int c, size_t n);

/* Seed for LFSR function used in rand() */
/* This seed was derived from running the LFSR with a seed of 1 - helps skip the
   first iterations which outputs the value shifting through. */
#define RAND_LFSR_SEED 0x14b6bc3c
/* Pseudo-random number generation */
unsigned long int rand ();

#endif
