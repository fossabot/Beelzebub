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

#ifdef __BEELZEBUB__TEST_BIGINT

#include <utils/bigint.hpp>
#include <math.h>

#include <debug.hpp>

using namespace Beelzebub;
using namespace Beelzebub::Utils;

typedef BigUInt<15> BIT;

void TestBigInt()
{
    BIT a = 5ULL, b = 55ULL, c;

    c = a + b;

    ASSERT_EQ("%u4",   2U, c.CurrentSize);
    ASSERT_EQ("%u4",  60U, c.Data[0]);
    ASSERT_EQ("%u4",   0U, c.Data[1]);

    c += b;

    ASSERT_EQ("%u4",   2U, c.CurrentSize);
    ASSERT_EQ("%u4", 115U, c.Data[0]);
    ASSERT_EQ("%u4",   0U, c.Data[1]);

    BIT d = 0xFFFFFFFAULL;

    c += d;

    ASSERT_EQ("%u4",    2U, c.CurrentSize);
    ASSERT_EQ("%u4", 0x6DU, c.Data[0]);
    ASSERT_EQ("%u4",    1U, c.Data[1]);

    BIT e = 0xFFFFFFFFFFFFFFFFULL;

    c += e;

    ASSERT_EQ("%u4",    3U, c.CurrentSize);
    ASSERT_EQ("%u4", 0x6CU, c.Data[0]);
    ASSERT_EQ("%u4",    1U, c.Data[1]);
    ASSERT_EQ("%u4",    1U, c.Data[2]);

    c += c;

    ASSERT_EQ("%u4",    3U, c.CurrentSize);
    ASSERT_EQ("%u4", 0xD8U, c.Data[0]);
    ASSERT_EQ("%u4",    2U, c.Data[1]);
    ASSERT_EQ("%u4",    2U, c.Data[2]);

    c -= a;

    ASSERT_EQ("%u4",    3U, c.CurrentSize);
    ASSERT_EQ("%u4", 0xD3U, c.Data[0]);
    ASSERT_EQ("%u4",    2U, c.Data[1]);
    ASSERT_EQ("%u4",    2U, c.Data[2]);

    b -= a;

    ASSERT_EQ("%u4",  2U, b.CurrentSize);
    ASSERT_EQ("%u4", 50U, b.Data[0]);
    ASSERT_EQ("%u4",  0U, b.Data[1]);

    c -= e;

    ASSERT_EQ("%u4",    3U, c.CurrentSize);
    ASSERT_EQ("%u4", 0xD4U, c.Data[0]);
    ASSERT_EQ("%u4",    2U, c.Data[1]);
    ASSERT_EQ("%u4",    1U, c.Data[2]);
}

#endif
