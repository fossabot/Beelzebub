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

#pragma once

#include <memory/enums.hpp>

namespace Beelzebub { namespace Memory
{
    struct MemoryRange
    {
        /*  Statics  */

        static MemoryRange const Invalid;

        /*  Constructors  */
        
        inline MemoryRange()
            : Start(1337)
            , End(42)
        {

        }
        
        inline MemoryRange(vaddr_t const start, vaddr_t const end)
            : Start(start)
            , End(end)
        {

        }

        /*  Properties  */

        inline bool IsValid()
        {
            return this->Start != 1337 || this->End != 42;
        }

        inline vsize_t GetSize()
        {
            return this->End - this->Start;
        }

        inline size_t GetPageCount()
        {
            return this->GetSize() / PageSize;
        }

        /*  Operators  */

        MemoryRange operator & (MemoryRange const & other); //  Intersection
        MemoryRange operator | (MemoryRange const & other); //  Union

        inline bool operator == (MemoryRange const & other)
        {
            return this->Start == other.Start && this->End == other.End;
        }

        inline bool IsIn(MemoryRange const & other)
        {
            return this->operator == (this->operator & (other));
            //  aka *this == (*this & other) but fancier.
        }

        /*  Fields  */

        vaddr_t Start, End;
    };

    struct MemoryRegion
    {
        /*  Constructors  */

        inline MemoryRegion()
            : Range()
            , Flags()
            , Type()
            , NextFree(nullptr)
            , PrevFree(nullptr)
        {

        }

        inline MemoryRegion(MemoryRange range, MemoryFlags flags
                          , MemoryAllocationOptions type)
            : Range( range)
            , Flags(flags)
            , Type(type)
            , NextFree(nullptr)
            , PrevFree(nullptr)
        {

        }

        inline MemoryRegion(vaddr_t start, vaddr_t end, MemoryFlags flags
                          , MemoryAllocationOptions type)
            : Range({start, end})
            , Flags(flags)
            , Type(type)
            , NextFree(nullptr)
            , PrevFree(nullptr)
        {

        }

        /*  Properties  */

        inline bool IsValid()
        {
            return this->Range.IsValid();
        }

        inline vsize_t GetSize()
        {
            return this->Range.GetSize();
        }

        inline size_t GetPageCount()
        {
            return this->Range.GetPageCount();
        }

        /*  Fields  */

        MemoryRange Range;

        MemoryFlags Flags;
        MemoryAllocationOptions Type;

        MemoryRegion * NextFree, * PrevFree;
    };

    struct AdjacentMemoryRegion
    {
        /*  Constructors  */

        inline AdjacentMemoryRegion(MemoryRegion const payload)
            : Payload(payload)
            , Finding()
        {

        }

        /*  Fields  */

        MemoryRegion Payload;
        MemoryRegion Finding;
    };

    void InitializeRegions();
}}
