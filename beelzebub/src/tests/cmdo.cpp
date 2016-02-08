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

#ifdef __BEELZEBUB__TEST_CMDO

#include <tests/cmdo.hpp>
#include <cmd_options.hpp>

#include <debug.hpp>

using namespace Beelzebub;

char OptionsString[] = "   --alpha='test a' -b \"yada yada\" --g\"amm\"a ra\\da --si\\erra -x  ";

CommandLineOptionParserState parser;

CMDO(1, "a", "alpha", String);
CMDO(2, "b", "beta", String);
CMDO(3, "c", "gamma", String);

CMDO(4, "s", "sierra", BooleanByPresence);
CMDO(5, "x", "xenulol", BooleanByPresence);

#define PARSE_OPT(name) do {                            \
res = parser.ParseOption(MCATS(CMDO_, name));           \
ASSERT(res.IsOkayResult()                               \
    , "Failed to parse command-line option \"%s\": %H"  \
    , #name, res);                                      \
} while (false)

void TestCmdo()
{
    Handle res;

    res = parser.StartParsing(OptionsString);

    ASSERT(res.IsOkayResult()
        , "Failed to start parsing command-line options: %H"
        , res);

    // for (size_t i = 0; i < parser.Length; ++i)
    //     if (parser.InputString[i] == '\0')
    //         parser.InputString[i] = '_';

    // msg("Length of \"%S\" is: %us.%n", parser.Length, parser.InputString, parser.Length);

    ASSERT(parser.TokenCount == 7
        , "Parser should have identified %us tokens, not %us."
        , 7, parser.TokenCount);

    PARSE_OPT(1);
    PARSE_OPT(2);
    PARSE_OPT(3);
    PARSE_OPT(4);
    PARSE_OPT(5);
}

#endif
