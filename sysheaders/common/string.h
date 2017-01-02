/*
    Copyright (c) 2015 Alexandru-Mihai Maftei. All rights reserved.


    Developed by: Alexandru-Mihai Maftei
    aka Vercas
    http://vercas.com | https://github.com/vercas/Beelzebub

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal with the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

      * Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimers.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimers in the
        documentation and/or other materials provided with the distribution.
      * Neither the names of Alexandru-Mihai Maftei, Vercas, nor the names of
        its contributors may be used to endorse or promote products derived from
        this Software without specific prior written permission.


    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    WITH THE SOFTWARE.

    ---

    You may also find the text of this license in "LICENSE.md", along with a more
    thorough explanation regarding other files.
*/

/** 
 *  Yeah, there are C++-specific declarations too.
 *  The C and common ones are defined in 'string.c'.
 *  The C++-specific ones are defined in 'string.cpp'.
 */

#pragma once

#include <beel/metaprogramming.h>

__shared_inline bool memeq(void const * src1, void const * src2, size_t len);
__shared_inline comp_t memcmp(void const * src1, void const * src2, size_t len);

__shared_inline void * memchr(void const * src, int val, size_t len);

__shared_inline void * memcpy(void * dst, void const * src, size_t len);
__shared_inline void * memmove(void * dst, void const * src, size_t len);
__shared_inline void * mempcpy(void * dst, void const * src, size_t len);
__shared_inline void * mempmove(void * dst, void const * src, size_t len);

__shared_inline void * memset(void * dst, int const val, size_t len);
__shared_inline void * mempset(void * dst, int const val, size_t len);
__shared_inline void * memset16(void * dst, int const val, size_t cnt);
__shared_inline void * mempset16(void * dst, int const val, size_t cnt);
__shared_inline void * memset32(void * dst, long const val, size_t cnt);
__shared_inline void * mempset32(void * dst, long const val, size_t cnt);

__shared_inline size_t strlen(char const * str);
__shared_inline size_t strnlen(char const * str, size_t len);
__shared_inline size_t strnlenex(char const * str, size_t len, bool * reached);

__shared_inline char * strcat(char * dst, char const * src);
__shared_inline char * strncat(char * dst, char const * src, size_t len);

__shared_inline char * strcpy(char * dst, char const * src);
__shared_inline char * strncpy(char * dst, char const * src, size_t len);

__shared_inline char * strpbrk(char const * haystack, char const * needle);
__shared_inline char * strnpbrk(char const * haystack, char const * needle, size_t len);

__shared_inline comp_t strcmp(char const * src1, char const * src2);
__shared_inline comp_t strncmp(char const * src1, char const * src2, size_t len);

__shared_inline comp_t strcasecmp(char const * src1, char const * src2);
__shared_inline comp_t strcasencmp(char const * src1, char const * src2, size_t len);

__shared_inline char * strchr(char const * haystack, int needle);
__shared_inline char * strstr(char const * haystack, char const * needle);

__shared_inline char const * strstrex(char const * haystack, char const * needle, char const * seps);
__shared_inline char const * strcasestrex(char const * haystack, char const * needle, char const * seps);

__shared char const * strerrorc(int errnum);
__shared char * strerror(int errnum);

#ifdef _GNU_SOURCE
__shared char * strerror_r(int errnum, char * buf, size_t buflen);
            /* GNU-specific */
#else
__shared int strerror_r(int errnum, char * buf, size_t buflen);
            /* XSI-compliant */
#endif
