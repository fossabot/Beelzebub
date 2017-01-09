/*
    Copyright (c) 2017 Alexandru-Mihai Maftei. All rights reserved.


    Developed by: Alexandru-Mihai Maftei
    aka Vercas
    http://vercas.com | https://github.com/vercas/Beelzebub

    Permission is hereby granted, Vmm::FreePages of charge, to any person obtaining a copy
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

#if defined(__BEELZEBUB__TEST_VMM)

#include "tests/vmm.hpp"
#include "memory/vmm.hpp"
#include "cores.hpp"
#include "kernel.hpp"
#include <new>

#include <beel/sync/smp.lock.hpp>
#include <math.h>
#include <debug.hpp>

#define PRINT

using namespace Beelzebub;
using namespace Beelzebub::Memory;
using namespace Beelzebub::Synchronization;
using namespace Beelzebub::System;

Barrier VmmTestBarrier;

#define SYNC VmmTestBarrier.Reach()

static SmpLock DeleteLock {};

static constexpr size_t const RandomIterations = 1'000;
static constexpr size_t const CacheSize = 2048;
static Atomic<vaddr_t> Cache[CacheSize];
static constexpr size_t const SyncerCount = 10;
static Atomic<vaddr_t> Syncers[SyncerCount];
static __thread vaddr_t MyCache[CacheSize];
#ifdef PRINT
static Atomic<size_t> RandomerCounter {0};
#endif
//  There is no space on the stack for this one.

void TestVmm(bool const bsp)
{
    if (bsp) Scheduling = false;

    size_t coreIndex = Cpu::GetData()->Index;

    auto getPtr = [](bool commit = true)
    {
        vaddr_t testptr = nullpaddr;

        Handle res = Vmm::AllocatePages(nullptr
            , 0x4000
            , commit
                ? (MemoryAllocationOptions::Commit | MemoryAllocationOptions::VirtualKernelHeap)
                : MemoryAllocationOptions::VirtualKernelHeap
            , MemoryFlags::Global | MemoryFlags::Writable
            , MemoryContent::Generic
            , testptr);

        ASSERTX(res == HandleResult::Okay)(res)XEND;
        ASSERTX(testptr >= Vmm::KernelStart
            , "Returned address (%Xp) is not in the kernel heap..?", testptr)XEND;

        return testptr;
    };

    auto delPtr = [](vaddr_t vaddr)
    {
        Handle res = Vmm::FreePages(nullptr, vaddr, 0x4000);

        ASSERTX(res == HandleResult::Okay)(res)XEND;
    };

#ifdef PRINT
    if (bsp) MSG_("Filling array.%n");
#endif

    SYNC;

#ifdef PRINT
    uint64_t perfStart = 0, perfEnd = 0;

    SYNC;

    perfStart = CpuInstructions::Rdtsc();
#endif

    vaddr_t cur = getPtr(), dummy = getPtr();

    for (size_t i = 0; i < CacheSize; ++i)
    {
        vaddr_t expected = nullpaddr;

        if ((Cache + i)->CmpXchgStrong(expected, cur))
            cur = getPtr();
    }

#ifdef PRINT
    perfEnd = CpuInstructions::Rdtsc();

    SYNC;

    MSG_("Core %us did filling in %us cycles: %us per slot.%n"
        , coreIndex, perfEnd - perfStart
        , (perfEnd - perfStart + CacheSize / 2 + 1) / (CacheSize + 2));

    SYNC;

    if (bsp) MSG_("First check.%n");
#endif

    for (size_t i = 0; i < CacheSize; ++i)
        ASSERT(cur != Cache[i]);

    SYNC;

#ifdef PRINT
    if (bsp) MSG_("Full diff check.%n");

    SYNC;
#endif

    for (size_t i = coreIndex; i < CacheSize; i += Cores::GetCount())
        for (size_t j = i + 1; j < CacheSize; ++j)
            ASSERT(Cache[i] != Cache[j]);

    SYNC;

    withLock (DeleteLock)
    {
#ifdef PRINT
        MSG_("Core %us frees %Xp.%n", coreIndex, cur);
#endif

        delPtr(cur);
    }

    cur = nullpaddr;

    SYNC;

#ifdef PRINT
    if (bsp) MSG_("Individual stability 1.%n");

    SYNC;

    perfStart = CpuInstructions::Rdtsc();
#endif

    for (size_t i = 0; i < RandomIterations; ++i)
        delPtr(getPtr());

#ifdef PRINT
    perfEnd = CpuInstructions::Rdtsc();

    MSG_("Core %us did %us Vmm::AllocatePages & Vmm::FreePages pairs in %us cycles; %us cycles per pair.%n"
        , coreIndex, RandomIterations, perfEnd - perfStart, (perfEnd - perfStart + RandomIterations / 2) / RandomIterations);

    SYNC;

    if (bsp) MSG_("Individual stability 2.%n");
#endif

    for (size_t i = 0; i < CacheSize; ++i)
        MyCache[i] = nullpaddr;

    SYNC;

#ifdef PRINT
    perfStart = CpuInstructions::Rdtsc();
#endif

    for (size_t i = 0, j = 0; j < RandomIterations; ++j)
    {
        if (MyCache[i] == nullpaddr)
            MyCache[i] = getPtr();
        else
        {
            delPtr(MyCache[i]);
            MyCache[i] = nullpaddr;
        }

        if (++i == CacheSize) i = 0;
    }

    for (size_t i = 0; i < CacheSize; ++i)
        if (MyCache[i] != nullpaddr)
            delPtr(MyCache[i]);

#ifdef PRINT
    perfEnd = CpuInstructions::Rdtsc();

    MSG_("Core %us did %us Vmm::AllocatePages & Vmm::FreePages in the same order in %us cycles; %us cycles per operation.%n"
        , coreIndex, RandomIterations, perfEnd - perfStart, (perfEnd - perfStart + RandomIterations / 2) / RandomIterations);
#endif

    SYNC;

#ifdef PRINT
    if (bsp) MSG_("Randomer 1!%n");

    SYNC;
#endif

    coreIndex ^= 0x55U;

#ifdef PRINT
    perfStart = CpuInstructions::Rdtsc();
#endif

    for (size_t i = 0, j = 0; j < RandomIterations; ++j)
    {
    retry:
        vaddr_t old = nullpaddr;

        if (Cache[i] != nullpaddr)
        {
            old = (Cache + i)->Xchg(old);

            if unlikely(old == nullpaddr)
                goto retry;
            //  If it became null in the meantime, retry.

            delPtr(old);
        }
        else
        {
            if (cur == nullpaddr)
                cur = getPtr();

            if likely((Cache + i)->CmpXchgStrong(old, cur))
                cur = nullpaddr;
            else
                goto retry;
            //  If it became non-null in the meantime, retry.
        }

        i += coreIndex;

        while (i >= CacheSize)
            i -= CacheSize;
    }

#ifdef PRINT
    perfEnd = CpuInstructions::Rdtsc();
#endif

    coreIndex ^= 0x55U;

    SYNC;

#ifdef PRINT
    RandomerCounter += perfEnd - perfStart;

    SYNC;

    if (bsp)
    {
        size_t const itcnt = Cores::GetCount() * RandomIterations;

        MSG_("%us cores did %us random Vmm::AllocatePages/Vmm::FreePages under random congestion in %us cycles; %us cycles per operation.%n"
            , Cores::GetCount(), itcnt, RandomerCounter.Load(), (RandomerCounter + itcnt / 2) / itcnt);
    }

    SYNC;
#endif

    if (bsp)
        for (size_t i = 0; i < CacheSize; ++i)
            if (Cache[i] == nullpaddr)
                Cache[i] = getPtr();

    if (cur != nullpaddr)
        delPtr(cur);

    SYNC;

#ifdef PRINT
    if (bsp)
    {
        MSG_("Randomer 2!%n");

        RandomerCounter.Store(0);
    }

    SYNC;
#endif

    coreIndex ^= 0x55U;

#ifdef PRINT
    perfStart = CpuInstructions::Rdtsc();
#endif

    for (size_t i = 0, j = 0; j < RandomIterations; ++j)
    {
        cur = getPtr();
        vaddr_t old = (Cache + i)->Xchg(cur);

        delPtr(old);

        i += coreIndex;

        while (i >= CacheSize)
            i -= CacheSize;
    }

#ifdef PRINT
    perfEnd = CpuInstructions::Rdtsc();
#endif

    coreIndex ^= 0x55U;

    SYNC;

#ifdef PRINT
    RandomerCounter += perfEnd - perfStart;

    SYNC;

    if (bsp)
    {
        size_t const itcnt = Cores::GetCount() * RandomIterations;

        MSG_("%us cores did %us random Vmm::AllocatePages/Vmm::FreePages under random congestion in %us cycles; %us cycles per operation.%n"
            , Cores::GetCount(), itcnt, RandomerCounter.Load(), (RandomerCounter + itcnt / 2) / itcnt);
    }

    SYNC;

    if (bsp) MSG_("Cleanup.%n");

    SYNC;
#endif

    if (bsp)
        for (size_t i = 0; i < CacheSize; ++i)
            if (Cache[i] != nullpaddr)
                delPtr(Cache[i]);

    delPtr(dummy);

    SYNC;

    if (bsp) Scheduling = true;
}

#endif
