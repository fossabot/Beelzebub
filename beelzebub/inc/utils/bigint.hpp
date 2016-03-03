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

#include <math.h>

namespace Beelzebub { namespace Utils
{
    bool BigIntAdd(uint32_t * dst, uint32_t const * src1, uint32_t const * src2, uint32_t size, bool cin);
    bool BigIntSub(uint32_t * dst, uint32_t const * src1, uint32_t const * src2, uint32_t size, bool cin);
    bool BigIntMul(uint32_t * dst, uint32_t const * src1, uint32_t const * src2, uint32_t size, bool cin);
    bool BigIntDiv(uint32_t * dst, uint32_t const * src1, uint32_t const * src2, uint32_t size, bool cin);
    bool BigIntMod(uint32_t * dst, uint32_t const * src1, uint32_t const * src2, uint32_t size, bool cin);

    template<uint32_t MaxSize>
    struct BigUInt
    {
        /*  Operations  */

        static uint32_t Balance(BigUInt && left, BigUInt && right)
        {
            if (left.CurrentSize > right.CurrentSize)
            {
                for (size_t i = right.CurrentSize; i < left.CurrentSize; ++i)
                    right.Data[i] = 0;

                return left.CurrentSize;
            }
            else
            {
                for (size_t i = left.CurrentSize; i < right.CurrentSize; ++i)
                    left.Data[i] = 0;

                return right.CurrentSize;
            }
        }

        /*  Constructors  */

        inline BigUInt()
            : Data({})
            , CurrentSize(0)
        {

        }

        inline BigUInt(uint32_t const val)
            : Data({val})
            , CurrentSize(1)
        {

        }

        inline BigUInt(uint64_t const val)
            : Data({val & 0xFFFFFFFF, val >> 32})
            , CurrentSize(2)
        {

        }

        /*  Operators  */

        inline BigUInt operator +(BigUInt && other)
        {
            BigUInt res {};

            BigIntAdd(&(res.Data[0]), &(this->Data[0]), &(other.Data[0])
                , res.CurrentSize = Balance(*this, other), false);

            return res;
        }

        inline BigUInt operator -(BigUInt && other)
        {
            BigUInt res {};

            BigIntSub(&(res.Data[0]), &(this->Data[0]), &(other.Data[0])
                , res.CurrentSize = Balance(*this, other), false);

            return res;
        }

        inline BigUInt operator *(BigUInt && other)
        {
            BigUInt res {};

            BigIntMul(&(res.Data[0]), &(this->Data[0]), &(other.Data[0])
                , res.CurrentSize = Balance(*this, other), false);

            return res;
        }

        inline BigUInt operator /(BigUInt && other)
        {
            BigUInt res {};

            BigIntDiv(&(res.Data[0]), &(this->Data[0]), &(other.Data[0])
                , res.CurrentSize = Balance(*this, other), false);

            return res;
        }

        inline BigUInt operator %(BigUInt && other)
        {
            BigUInt res {};

            BigIntMod(&(res.Data[0]), &(this->Data[0]), &(other.Data[0])
                , res.CurrentSize = Balance(*this, other), false);

            return res;
        }

        /*  Fields  */

        uint32_t Data[MaxSize];
        uint32_t CurrentSize;
    } __aligned(16);
}}
