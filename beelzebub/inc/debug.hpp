#pragma once

#include "stdarg.h"

#include <metaprogramming.h>
#include <handles.h>
#include <synchronization/spinlock.hpp>

#include <kernel.hpp>

#ifdef __BEELZEBUB__DEBUG
#define assert(cond, ...) do {                                          \
if unlikely(!(cond))                                                    \
    Beelzebub::Debug::CatchFireFormat(__FILE__, __LINE__, __VA_ARGS__); \
} while (false)

//#define assert(cond, msg) Beelzebub::Debug::Assert(cond, __FILE__, __LINE__, msg)
#define msg(...) do {                                                   \
    if likely(Beelzebub::MainTerminal != nullptr)                       \
    {                                                                   \
        (&Beelzebub::Debug::MsgSpinlock)->Acquire();                    \
        Beelzebub::MainTerminal->WriteFormat(__VA_ARGS__);              \
        (&Beelzebub::Debug::MsgSpinlock)->Release();                    \
    }                                                                   \
} while (false)
#else
#define assert(...) do {} while(false)
#define msg(...) do {} while(false)
#endif

using namespace Beelzebub::Synchronization;

namespace Beelzebub { namespace Debug
{
    extern Spinlock MsgSpinlock;

    __cold __bland __noreturn void CatchFire(const char * const file
                                           , const size_t line
                                           , const char * const msg);

    __cold __bland __noreturn void CatchFire(const char * const file
                                           , const size_t line
                                           , const char * const fmt, va_list args);

    __cold __bland __noreturn void CatchFireFormat(const char * const file
                                                 , const size_t line
                                                 , const char * const fmt, ...);

    __bland void Assert(const bool condition
                      , const char * const file
                      , const size_t line
                      , const char * const msg);

    __bland void Assert(const bool condition
                      , const char * const file
                      , const size_t line
                      , const char * const msg, va_list args);

    __bland void AssertFormat(const bool condition
                            , const char * const file
                            , const size_t line
                            , const char * const fmt, ...);
}}

