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

#include <terminals/interface.hpp>
#include <metaprogramming.h>
#include <handles.h>

#include "stdarg.h"

#define TERMTRY0(call, tres) do {           \
tres = call;                                \
if (!tres.Result.IsOkayResult())            \
	return tres;                            \
} while (false)

#define TERMTRY1(call, tres, cnt) do {      \
cnt = tres.Size;                            \
tres = call;                                \
if (tres.Result.IsOkayResult())             \
	tres.Size += cnt;                       \
else                                        \
	return tres;                            \
} while (false)

#define TERMTRY2(n, call, tres, cnt) do {     \
cnt = tres.Size;                              \
for (typeof(n) i = 0; i < n; ++i)             \
	if (!(tres = call).Result.IsOkayResult()) \
		return tres;                          \
tres.Size += cnt + n;                         \
} while (false)

namespace Beelzebub { namespace Terminals
{
	/**
	 * Represents a set of terminal capabilities.
	 **/
	class TerminalBase
	{
	public:

		/*  STATICS  */

		static const uint16_t DefaultTabulatorWidth = 8;

		/*  Writing  */

		static __bland TerminalWriteResult DefaultWriteCharAtXy(TerminalBase * const term, const char c, const int16_t x, const int16_t y);
		static __bland TerminalWriteResult DefaultWriteCharAtCoords(TerminalBase * const term, const char c, const TerminalCoordinates pos);
		static __bland TerminalWriteResult DefaultWriteStringAt(TerminalBase * const term, const char * const str, const TerminalCoordinates pos);

		static __bland TerminalWriteResult DefaultWriteChar(TerminalBase * const term, const char c);
		static __bland TerminalWriteResult DefaultWriteString(TerminalBase * const term, const char * const str);
		static __bland TerminalWriteResult DefaultWriteStringVarargs(TerminalBase * const term, const char * const fmt, va_list args);
		static __bland TerminalWriteResult DefaultWriteStringLine(TerminalBase * const term, const char * const str);
		static __bland TerminalWriteResult DefaultWriteStringFormat(TerminalBase * const term, const char * const fmt, ...);

		/*  Positioning  */

		static __bland Handle DefaultSetCursorPositionXy(TerminalBase * const term, const int16_t x, const int16_t y);
		static __bland Handle DefaultSetCursorPositionCoords(TerminalBase * const term, const TerminalCoordinates pos);
		static __bland TerminalCoordinates DefaultGetCursorPosition(TerminalBase * const term);

		static __bland Handle DefaultSetCurrentPositionXy(TerminalBase * const term, const int16_t x, const int16_t y);
		static __bland Handle DefaultSetCurrentPositionCoords(TerminalBase * const term, const TerminalCoordinates pos);
		static __bland TerminalCoordinates DefaultGetCurrentPosition(TerminalBase * const term);

		static __bland Handle DefaultSetSizeXy(TerminalBase * const term, const int16_t w, const int16_t h);
		static __bland Handle DefaultSetSizeCoords(TerminalBase * const term, const TerminalCoordinates pos);
		static __bland TerminalCoordinates DefaultGetSize(TerminalBase * const term);

		static __bland Handle DefaultSetBufferSizeXy(TerminalBase * const term, const int16_t w, const int16_t h);
		static __bland Handle DefaultSetBufferSizeCoords(TerminalBase * const term, const TerminalCoordinates pos);
		static __bland TerminalCoordinates DefaultGetBufferSize(TerminalBase * const term);

		static __bland Handle DefaultSetTabulatorWidth(TerminalBase * const term, const uint16_t w);
		static __bland uint16_t DefaultGetTabulatorWidth(TerminalBase * const term);

		/*  Styling  */

		// ... sooooon.
		
		/*  DYNAMICS  */

		/*  Descriptor  */

		const TerminalDescriptor * Descriptor;

		/*  Constructor  */

		__bland TerminalBase(const TerminalDescriptor * const desc);

		/*  Writing  */

		__bland TerminalWriteResult WriteAt(const char c, const int16_t x, const int16_t y);
		__bland TerminalWriteResult WriteAt(const char c, const TerminalCoordinates pos);
		__bland TerminalWriteResult WriteAt(const char * const str, const TerminalCoordinates pos);

		__bland TerminalWriteResult Write(const char c);
		__bland TerminalWriteResult Write(const char * const str);
		__bland TerminalWriteResult Write(const char * const fmt, va_list args);
		__bland TerminalWriteResult WriteLine(const char * const str);
		__bland __noinline TerminalWriteResult WriteFormat(const char * const fmt, ...);

		/*  Positioning  */

		__bland Handle SetCursorPosition(const int16_t x, const int16_t y);
		__bland Handle SetCursorPosition(const TerminalCoordinates pos);
		__bland TerminalCoordinates GetCursorPosition();

		__bland Handle SetCurrentPosition(const int16_t x, const int16_t y);
		__bland Handle SetCurrentPosition(const TerminalCoordinates pos);
		__bland TerminalCoordinates GetCurrentPosition();

		__bland Handle SetSize(const int16_t w, const int16_t h);
		__bland Handle SetSize(const TerminalCoordinates pos);
		__bland TerminalCoordinates GetSize();

		__bland Handle SetBufferSize(const int16_t w, const int16_t h);
		__bland Handle SetBufferSize(const TerminalCoordinates pos);
		__bland TerminalCoordinates GetBufferSize();

		__bland Handle SetTabulatorWidth(const uint16_t w);
		__bland uint16_t GetTabulatorWidth();

		/*	Utilitary methods  */

		__bland TerminalWriteResult WriteHandle(const Handle val);

		__bland TerminalWriteResult WriteIntD(const int64_t val);
		__bland TerminalWriteResult WriteUIntD(const uint64_t val);

		__bland TerminalWriteResult WriteHex8(const uint8_t val);
		__bland TerminalWriteResult WriteHex16(const uint16_t val);
		__bland TerminalWriteResult WriteHex32(const uint32_t val);
		__bland TerminalWriteResult WriteHex64(const uint64_t val);

		__bland TerminalWriteResult WriteHexDump(const uintptr_t start, const size_t length, const size_t charsPerLine);
		__bland __forceinline TerminalWriteResult WriteHexDump(const void * const start, const size_t length, const size_t charsPerLine)
		{
			return this->WriteHexDump((uintptr_t)start, length, charsPerLine);
		}

		__bland TerminalWriteResult WriteHexTable(const uintptr_t start, const size_t length, const size_t charsPerLine, const bool ascii);
		__bland __forceinline TerminalWriteResult WriteHexTable(const void * const start, const size_t length, const size_t charsPerLine, const bool ascii)
		{
			return this->WriteHexTable((uintptr_t)start, length, charsPerLine, ascii);
		}

		__bland __forceinline TerminalWriteResult WriteLine()
		{
			return this->WriteLine("");
		}

	protected:

		TerminalCoordinates CurrentPosition;
		uint16_t TabulatorWidth;
		bool Overflown;
	};
}}
