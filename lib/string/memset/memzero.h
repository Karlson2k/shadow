// SPDX-FileCopyrightText: 2022-2023, Christian Göttsche <cgzones@googlemail.com>
// SPDX-FileCopyrightText: 2023-2024, Alejandro Colomar <alx@kernel.org>
// SPDX-License-Identifier: BSD-3-Clause


#ifndef SHADOW_INCLUDE_LIB_STRING_MEMSET_MEMZERO_H_
#define SHADOW_INCLUDE_LIB_STRING_MEMSET_MEMZERO_H_


#include "config.h"

#include <stddef.h>
#include <string.h>
#include <strings.h>

#include "sizeof.h"


#define MEMZERO(arr)  memzero(arr, SIZEOF_ARRAY(arr))


inline void *memzero(void *ptr, size_t size);
inline char *strzero(char *s);


inline void *
memzero(void *ptr, size_t size)
{
#if defined(HAVE_MEMSET_EXPLICIT)
	memset_explicit(ptr, 0, size);
#elif defined(HAVE_EXPLICIT_BZERO)
	explicit_bzero(ptr, size);
#else
	bzero(ptr, size);
	__asm__ __volatile__ ("" : : "r"(ptr) : "memory");
#endif
	return ptr;
}


inline char *
strzero(char *s)
{
	return memzero(s, strlen(s));
}


#endif  // include guard
