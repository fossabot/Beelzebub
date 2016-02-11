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

#include <handles.h>

#define CMDO_LINKED(name, sf, lf, vt, other)        \
CommandLineOptionSpecification MCATS(CMDO_, name)   \
{                                                   \
    sf,                                             \
    lf,                                             \
    CommandLineOptionValueTypes::vt,                \
    &(MCATS(CMDO_, other))                          \
}

#define CMDO_LINKED2(name, sf, lf, vt, other)       \
CommandLineOptionSpecification MCATS(CMDO_, name)   \
{                                                   \
    sf,                                             \
    lf,                                             \
    CommandLineOptionValueTypes::vt,                \
    other                                           \
}

#define CMDO(name, sf, lf, vt)                      \
CommandLineOptionSpecification MCATS(CMDO_, name)   \
{                                                   \
    sf,                                             \
    lf,                                             \
    CommandLineOptionValueTypes::vt,                \
    nullptr                                         \
}

#define CMDO_LINKED_EX(name, sf, lf, vt, other)     \
MCATS(CMDO_, name) = CommandLineOptionSpecification \
(                                                   \
    sf,                                             \
    lf,                                             \
    CommandLineOptionValueTypes::vt,                \
    &(MCATS(CMDO_, other))                          \
)

#define CMDO_LINKED2_EX(name, sf, lf, vt, other)    \
MCATS(CMDO_, name) = CommandLineOptionSpecification \
(                                                   \
    sf,                                             \
    lf,                                             \
    CommandLineOptionValueTypes::vt,                \
    other                                           \
)

#define CMDO_EX(name, sf, lf, vt)                   \
MCATS(CMDO_, name) = CommandLineOptionSpecification \
(                                                   \
    sf,                                             \
    lf,                                             \
    CommandLineOptionValueTypes::vt,                \
    nullptr                                         \
)

namespace Beelzebub
{
    /**
     *  Possible types of command-line option values.
     */
    enum class CommandLineOptionValueTypes
        : uintptr_t
    {
        BooleanByPresence = 0,
        BooleanExplicit = 1,
        String = 2,
        SignedInteger = 3,
        UnsignedInteger = 4,
        Float = 5,
    };

    /**
     *  Represents specification for parsing command line options.
     */
    struct CommandLineOptionSpecification
    {
    public:
        /*  Constructors  */

        inline CommandLineOptionSpecification()
            : ShortForm(nullptr)
            , LongForm(nullptr)
            , ValueType()
            , ParsingResult()
            , StringValue(nullptr)
            , Next(nullptr)
        {

        }

        inline CommandLineOptionSpecification(char const * const sf
                                            , char const * const lf
                                            , CommandLineOptionValueTypes const vt
                                            , CommandLineOptionSpecification * const next)
            : ShortForm(sf)
            , LongForm(lf)
            , ValueType(vt)
            , ParsingResult()
            , StringValue(nullptr)
            , Next(next)
        {

        }

        /*  Fields  */

        char const * ShortForm;
        char const * LongForm;

        CommandLineOptionValueTypes ValueType;

        Handle ParsingResult;

        union
        {
            char *   StringValue;
            bool     BooleanValue;
            int64_t  SignedIntegerValue;
            uint64_t UnsignedIntegerValue;
        };

        CommandLineOptionSpecification * Next;
    };

    /**
     *  Represents the state of a command-line option parser.
     */
    class CommandLineOptionBatchState;

    /**
     *  Represents the state of a command-line option parser.
     */
    class CommandLineOptionParser
    {
    public:
        /*  Constructor(s)  */

        inline CommandLineOptionParser()
            : InputString(nullptr)
            , Length(0)
            , TokenCount(0)
            , Started(false)
        {
            //  Nuthin'.
        }

        /*  Operations  */

        Handle StartParsing(char * const input);

        Handle ParseOption(CommandLineOptionSpecification & opt);

        Handle StartBatch(CommandLineOptionBatchState    & state
                        , CommandLineOptionSpecification * head);

        /*  Fields  */

        char * InputString;
        size_t Length;
        size_t TokenCount;

        bool Started;
    };

    /**
     *  The steps taken in processing a batch of command-line options.
     */
    enum class CommandLineOptionBatchStep
    {
        Starting = 0,
        InProgress = 1,
        WrappingUp = 2,
        Finished = 4,

        Invalid = -1,
    };

    class CommandLineOptionBatchState
    {
    public:
        /*  Constructors  */

        inline CommandLineOptionBatchState()
            : Parser(nullptr)
            , Head(nullptr)
            , Offset(~((size_t)0))
            , Step(CommandLineOptionBatchStep::Invalid)
            , Result()
        {
            //  Nuthin'.
        }

        inline CommandLineOptionBatchState(CommandLineOptionParser * parser
                                         , CommandLineOptionSpecification * head)
            : Parser(parser)
            , Head(head)
            , Offset(0)
            , Step(CommandLineOptionBatchStep::Starting)
            , Result()
        {
            //  Nuthin' here eitha'.
        }

        /*  Operations  */

        Handle Next();

        /*  Properties  */

        __forceinline bool IsValid()
        {
            return this->Step != CommandLineOptionBatchStep::Invalid
                && this->Head != nullptr && this->Parser != nullptr
                && this->Offset != ~((size_t)0);
        }

        __forceinline bool IsFinished()
        {
            return this->Step == CommandLineOptionBatchStep::Finished;
        }

        /*  Fields  */

        CommandLineOptionParser const * const Parser;
        CommandLineOptionSpecification * const Head;

        size_t Offset;

        CommandLineOptionBatchStep Step;

        Handle Result;
    };
}
