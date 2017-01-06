/*
    Copyright (c) 2017 Alexandru-Mihai Maftei. All rights reserved.


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

#define VALLOC_SOURCE

#include <valloc/interface.hpp>
#include <valloc/platform.hpp>
#include <stdlib.h>
#include <string.h>

using namespace Valloc;

void * malloc(size_t size)
{
    return AllocateMemory(size);
}

void * calloc(size_t cnt, size_t size)
{
    size_t const totalSize = cnt * size;
    void * const ret = AllocateMemory(totalSize);

    if (VALLOC_LIKELY(ret != nullptr))
        memset(ret, 0, totalSize);

    return ret;
}

void * valloc(size_t size)
{
    return AllocateAlignedMemory(size, Platform::PageSize, 0);
}

void * memalign(size_t boundary, size_t size)
{
    return AllocateAlignedMemory(size, boundary, 0);
}

// int posix_memalign(void * * memptr, size_t alignment, size_t size) { return je_posix_memalign(memptr, alignment, size); }

void * realloc(void * ptr, size_t size)
{
    if (VALLOC_LIKELY(ptr != nullptr && size > 0))
        return ResizeAllocation(ptr, size);
    else if (ptr == nullptr)
        return AllocateMemory(size);
    else
    {
        DeallocateMemory(ptr);

        return nullptr;
    }
}

void free(void * ptr)
{
    if (VALLOC_LIKELY(ptr != nullptr))
        return DeallocateMemory(ptr);
}


