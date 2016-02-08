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

/*
    The input string must be prepared a little before it can be used efficiently.
    This is done with a two-pass algorithm.

    The first pass identifies the different elements of the input. An element is
    a component of an option. Either its name, its value or both.

    The second pass sanitizes the individual tokens so they can be used more
    quickly and efficiently by the individual option parsing function.
    Sanitation means expanding escape sequences and compacting the elements
    (removing multiplicate null terminators).
*/

#include <cmd_options.hpp>

using namespace Beelzebub;

/*****************************************
    CommandLineOptionParserState class
*****************************************/

/*  Operations  */

Handle CommandLineOptionParserState::StartParsing(char * const input)
{
    if (this->Started)
        return HandleResult::UnsupportedOperation;

    size_t len;

    this->InputString = input;
    len = strlen(input);

    //  First pass.

    bool underEscape = false, underSingleQuotes = false, underDoubleQuotes = false;

    for (size_t i = 0; i < len; ++i)
    {
        if (underEscape)
        {
            //  Nothing to do here at this step.

            underEscape = false;
        }
        else if (underSingleQuotes)
        {
            switch (input[i])
            {
                case '\\':
                    underEscape = true;
                    break;

                case '\'':
                    underSingleQuotes = false;
                    break;
            }
        }
        else if (underDoubleQuotes)
        {
            switch (input[i])
            {
                case '\\':
                    underEscape = true;
                    break;

                case '"':
                    underDoubleQuotes = false;
                    break;
            }
        }
        else
        {
            switch (input[i])
            {
                case '\\':
                    underEscape = true;
                    break;

                case '\'':
                    underSingleQuotes = true;
                    break;

                case '"':
                    underDoubleQuotes = true;
                    break;

                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    input[i] = '\0';
                    break;
            }
        }
    }

    if (underEscape || underSingleQuotes || underDoubleQuotes)
        return HandleResult::CmdOptionsMalformatted;

    //  Second pass.

    size_t j = 0, tkCnt = 0;
    bool lastWasNull = true;

    for (size_t i = 0; i < len; ++i)
    {
        if (underEscape)
        {
            underEscape = false;

            switch (input[i])
            {
                case '0':
                    input[j++] = '\0';
                    break;

                case 'a':
                    input[j++] = '\a';
                    break;

                case 'b':
                    input[j++] = '\b';
                    break;

                case 'f':
                    input[j++] = '\f';
                    break;

                case 'n':
                    input[j++] = '\n';
                    break;

                case 'r':
                    input[j++] = '\r';
                    break;

                case 'v':
                    input[j++] = '\v';
                    break;

                case 't':
                    input[j++] = '\t';
                    break;

                default:
                    input[j++] = input[i];
                    break;
            }
        }
        else if (underSingleQuotes)
        {
            switch (input[i])
            {
                case '\\':
                    underEscape = true;
                    break;

                case '\'':
                    underSingleQuotes = false;
                    break;

                default:
                    input[j++] = input[i];
                    break;
            }
        }
        else if (underDoubleQuotes)
        {
            switch (input[i])
            {
                case '\\':
                    underEscape = true;
                    break;

                case '"':
                    underDoubleQuotes = false;
                    break;

                default:
                    input[j++] = input[i];
                    break;
            }
        }
        else
        {
            if (input[i] == '\0')
            {
                if (!lastWasNull)
                {
                    lastWasNull = true;

                    input[j++] = input[i];

                    ++tkCnt;
                }

                //  Else do nuthin'.
            }
            else
            {
                lastWasNull = false;

                switch (input[i])
                {
                    case '\\':
                        underEscape = true;
                        break;

                    case '\'':
                        underSingleQuotes = true;
                        break;

                    case '"':
                        underDoubleQuotes = true;
                        break;

                    default:
                        input[j++] = input[i];
                        break;
                }
            }
        }
    }

    if (!lastWasNull)
    {
        input[j] = '\0';

        ++tkCnt;
    }
    else
        --j;

    this->Length = j;
    this->TokenCount = tkCnt;

    //  Done.

    this->Started = true;

    return HandleResult::Okay;
}

Handle CommandLineOptionParserState::ParseOption(CommandLineOptionSpecification & opt)
{


    return HandleResult::Okay;
}
