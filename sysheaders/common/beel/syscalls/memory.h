/*
    Copyright (c) 2016 Alexandru-Mihai Maftei. All rights reserved.


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

#pragma once

#include <beel/handles.h>

#define __ENUM_MEMREQOPTS(ENUMINST) \
    ENUMINST(None       , MEMREQ_NONE        , 0x000, "None"        ) \
    ENUMINST(Writable   , MEMREQ_WRITABLE    , 0x001, "Writable"    ) \
    ENUMINST(Executable , MEMREQ_EXECUTABLE  , 0x002, "Executable"  ) \
    ENUMINST(GuardLow   , MEMREQ_GUARD_LOW   , 0x010, "Guard Low"   ) \
    ENUMINST(GuardHigh  , MEMREQ_GUARD_HIGH  , 0x020, "Guard High"  ) \
    ENUMINST(Reserve    , MEMREQ_RESERVE     , 0x100, "Reserve"     ) \
    ENUMINST(Commit     , MEMREQ_COMMIT      , 0x200, "Commit"      ) \
    ENUMINST(ThreadStack, MEMREQ_THREAD_STACK, 0x031, "Thread Stack")

#define __ENUM_MEMRELOPTS(ENUMINST) \
    ENUMINST(None       , MEMREL_NONE        , 0x000, "None"        ) \
    ENUMINST(Decommit   , MEMREL_DECOMMIT    , 0x001, "Decommit"    )

__PUB_ENUM(MemoryRequestOptions, __ENUM_MEMREQOPTS, FULL)
__PUB_ENUM(MemoryReleaseOptions, __ENUM_MEMRELOPTS, FULL)

__PUB_FUNC(BeHandle, MemoryRequest, uintptr_t addr, size_t    size, BeMemoryRequestOptions opts);
__PUB_FUNC(BeHandle, MemoryRelease, uintptr_t addr, size_t    size, BeMemoryReleaseOptions opts);
__PUB_FUNC(BeHandle, MemoryCopy   , uintptr_t dst , uintptr_t src , size_t                 len );
__PUB_FUNC(BeHandle, MemoryFill   , uintptr_t dst , uint8_t   val , size_t                 len );
