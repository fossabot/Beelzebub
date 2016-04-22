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

#include <memory/vas.hpp>
#include <system/interrupts.hpp>

#include <debug.hpp>

using namespace Beelzebub;
using namespace Beelzebub::Memory;
using namespace Beelzebub::Synchronization;
using namespace Beelzebub::Utils;

/****************
    VAS class
****************/

/*  Constructors  */

Handle Vas::Initialize(vaddr_t start, vaddr_t end
    , AcquirePoolFunc acquirer, EnlargePoolFunc enlarger, ReleasePoolFunc releaser
    , PoolReleaseOptions const releaseOptions
    , size_t const quota)
{
    new (&(this->Alloc)) ObjectAllocator(
        sizeof(*(this->Tree.Root)), __alignof(*(this->Tree.Root)),
        acquirer, enlarger, releaser, releaseOptions, SIZE_MAX, quota);

    return this->Tree.Insert(MemoryRegion(start, end
        , MemoryFlags::Writable | MemoryFlags::Executable
        , MemoryAllocationOptions::Free), this->FirstFree);
    //  Blank memory region, for allocation.
}

/*  Operations  */

Handle Vas::Allocate(vaddr_t & vaddr, size_t pageCnt
    , MemoryFlags flags, MemoryAllocationOptions type)
{
    if unlikely(this->FirstFree == nullptr)
        return HandleResult::ObjectDisposed;

    Handle res = HandleResult::Okay;

    size_t lowOffset = 0, highOffset = 0;
    size_t effectivePageCnt = pageCnt;

    if (0 != (type & MemoryAllocationOptions::GuardLow))
    {
        lowOffset = PageSize;
        ++effectivePageCnt;
    }

    if (0 != (type & MemoryAllocationOptions::GuardHigh))
    {
        highOffset = PageSize;
        ++effectivePageCnt;
    }

    System::InterruptGuard<> intGuard;
    //  Guard the rest of the scope from interrupts.

    this->Lock.AcquireAsWriter();

    DEBUG_TERM << " <ALLOC> ";

    if (vaddr == nullvaddr)
    {
        //  Null vaddr means any address is accepted.

        DEBUG_TERM << " <NULL VADDR> ";

        MemoryRegion * reg = this->FirstFree;

        do
        {
            DEBUG_TERM << *reg;

            size_t regPageCnt = reg->GetPageCount();

            if (regPageCnt > effectivePageCnt)
            {
                //  Oh yush, this region contains more than enough pages!

                DEBUG_TERM << " <BIGGER> ";

                //  TODO: Implement (K)ASLR here.
                //  For now, it allocates at the end of the space, so it doesn't
                //  slow down future allocations in any meaningful way.

                MemoryRegion * newReg = nullptr;
                vaddr_t const newRegEnd = reg->Range.End;

                res = this->Tree.Insert(MemoryRegion(
                    reg->Range.End -= effectivePageCnt * PageSize,
                    newRegEnd, flags, type
                ), newReg);
                //  New region begins where the free one ends, after shrinking.

                if unlikely(!res.IsOkayResult())
                    goto end;
                //  Disastrous allocation failure could occur. ;-;

                DEBUG_TERM << *newReg;

                vaddr = newReg->Range.Start + lowOffset;

                newReg->PrevFree = reg;
                newReg->NextFree = reg->NextFree;
                //  Complete linking.

                goto end;
            }
            else
            {
                //  This region appears to be an exact fit.
                
                DEBUG_TERM << " <SNUG> ";

                if (reg->PrevFree == nullptr)
                {
                    //  This is the first region.

                    this->FirstFree = reg->NextFree;
                }
                else
                    reg->PrevFree->NextFree = reg->NextFree;

                if (reg->NextFree != nullptr)
                    reg->NextFree->PrevFree = reg->PrevFree;

                //  Now linkage is patched, and the region can be merrily
                //  converted!

                reg->Flags = flags;
                reg->Type = type;

                vaddr = reg->Range.Start;

                goto end;
                //  Done!
            }

            //  Only other possibility is... This free region isn't big enough!

            reg = reg->NextFree;
        } while (reg != nullptr);

        //  Reaching this point means there is no space to spare!

        res = HandleResult::OutOfMemory;
    }
    else
    {
        //  A non-null vaddr means a specific address is required. Guard would
        //  go before the requested address.

        DEBUG_TERM << " <VADDR " << (void *)vaddr << "> ";

        MemoryRange rang {vaddr - lowOffset, vaddr + pageCnt * PageSize + highOffset};
        //  This will be the exact range of the allocation.

        MemoryRegion * reg = this->FirstFree;

        do
        {
            DEBUG_TERM << *reg;

            size_t regPageCnt = reg->GetPageCount();

            if (!rang.IsIn(reg->Range) || effectivePageCnt > regPageCnt)
            {
                reg = reg->NextFree;

                continue;
            }

            //  So it fits!

            if (regPageCnt == effectivePageCnt)
            {
                //  This region appears to be an exact fit. What a relief, and
                //  coincidence!
                
                DEBUG_TERM << " <SNUG> ";

                if (reg->PrevFree == nullptr)
                {
                    //  This is the first region.

                    this->FirstFree = reg->NextFree;
                }
                else
                    reg->PrevFree->NextFree = reg->NextFree;

                if (reg->NextFree != nullptr)
                    reg->NextFree->PrevFree = reg->PrevFree;

                //  Now linkage is patched, and the region can be merrily
                //  converted!

                reg->Flags = flags;
                reg->Type = type;

                vaddr = reg->Range.Start;

                goto end;
                //  Done!
            }

            //  Okay, so it's not a perfect fit... Meh. Split.
            //  There are 3 possibilities: the desired range is at the start of
            //  the region, at the end, or in the middle...

            if (rang.Start == reg->Range.Start)
            {
                //  So it sits at the very start.

                DEBUG_TERM << " <START> ";

                MemoryRegion * newReg = nullptr;

                res = this->Tree.Insert(MemoryRegion(
                    rang.Start, reg->Range.Start = rang.End, flags, type
                ), newReg);
                //  Free region's start is pushed.

                if unlikely(!res.IsOkayResult())
                    goto end;

                DEBUG_TERM << *newReg;

                newReg->PrevFree = reg->PrevFree;
                newReg->NextFree = reg;

                goto end;
            }
            else if (rang.End == reg->Range.End)
            {
                //  Or at the end.

                DEBUG_TERM << " <END> ";

                MemoryRegion * newReg = nullptr;

                res = this->Tree.Insert(MemoryRegion(
                    reg->Range.End = rang.Start, rang.End, flags, type
                ), newReg);
                //  Free region's end is pulled.

                if unlikely(!res.IsOkayResult())
                    goto end;

                DEBUG_TERM << *newReg;

                newReg->PrevFree = reg;
                newReg->NextFree = reg->NextFree;

                goto end;
            }
            else
            {
                //  Well, it's in the middle.

                DEBUG_TERM << " <MID> ";

                vaddr_t const oldEnd = reg->Range.End;
                reg->Range.End = rang.Start;
                //  Pulls the end of the free region to become the left free region.

                MemoryRegion * newFree = nullptr,  * newBusy = nullptr;

                res = this->Tree.Insert(MemoryRegion(
                    rang.End, oldEnd, flags, type
                ), newFree);

                if unlikely(!res.IsOkayResult())
                    goto end;

                res = this->Tree.Insert(MemoryRegion(
                    rang.Start, rang.End, flags, type
                ), newBusy);

                if unlikely(!res.IsOkayResult())
                    goto end;

                DEBUG_TERM << *newFree << " " << *newBusy;

                newFree->PrevFree = reg;
                newFree->NextFree = reg->NextFree;
                reg->NextFree = reg;

                newBusy->PrevFree = reg;
                newBusy->NextFree = newFree;

                goto end;
            }
        } while (reg != nullptr);

        //  Reaching this point means the requested region is taken!

        res = HandleResult::OutOfMemory;
    }

end:
    this->Lock.ReleaseAsWriter();

    return res;
}

MemoryRegion * Vas::FindRegion(vaddr_t vaddr, bool keepLock)
{
    this->Lock.AcquireAsReader();

    MemoryRegion * reg = this->Tree.Find<vaddr_t>(vaddr);

    if unlikely(!keepLock)
        this->Lock.ReleaseAsReader();

    return reg;
}

/*************
    OTHERS
*************/

namespace Beelzebub { namespace Utils
{
    template<>
    Handle AvlTree<MemoryRegion>::AllocateNode(AvlTree<MemoryRegion>::Node * & node, void * cookie)
    {
        return (reinterpret_cast<ObjectAllocator *>(cookie))->AllocateObject(node);
    }

    template<>
    Handle AvlTree<MemoryRegion>::RemoveNode(AvlTree<MemoryRegion>::Node * const node, void * cookie)
    {
        return (reinterpret_cast<ObjectAllocator *>(cookie))->DeallocateObject(node);
    }
}}
