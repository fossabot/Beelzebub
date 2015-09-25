#pragma once

#include <metaprogramming.h>

#ifdef __cplusplus

#ifdef __BEELZEBUB__ARCH_X86
template<typename TNum1, typename TNum2>
__bland __forceinline __const constexpr auto RoundUp(const TNum1 value, const TNum2 step) -> decltype(((value + step - 1) / step) * step)
{
    return ((value + step - 1) / step) * step;
}
#else
template<typename TNum1, typename TNum2>
__bland __forceinline __const constexpr auto RoundUp(const TNum1 value, const TNum2 step) -> decltype(value + ((step - (value % step)) % step))
{
    return value + ((step - (value % step)) % step);
}
#endif

template<typename TNum1, typename TNum2>
__bland __forceinline __const constexpr auto RoundDown(const TNum1 value, const TNum2 step) -> decltype(value - (value & step))
{
    return value - (value & step);
}

template<typename TNum1, typename TNum2>
__bland __forceinline __const constexpr auto RoundUpDiff(const TNum1 value, const TNum2 step) -> decltype((step - (value % step)) % step)
{
    return (step - (value % step)) % step;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

template<typename TNum1>
__bland __forceinline __const constexpr TNum1 Minimum(const TNum1 & a)
{
    return a;
}
template<typename TNum1, typename TNum2>
__bland __forceinline __const constexpr auto Minimum(const TNum1 & a, const TNum2 & b) -> decltype((a < b) ? a : b)
{
    return (a < b) ? a : b;
}
template<typename TNum1, typename TNum2, typename TNum3>
__bland __forceinline __const constexpr auto Minimum(const TNum1 & a, const TNum2 & b, const TNum3 & c) -> decltype((a < Minimum(b, c)) ? a : Minimum(b, c))
{
    auto _b = Minimum(b, c);

    return (a < _b) ? a : _b;
}

/*template<typename TNum1, typename ... TNumOthers>
__bland __forceinline __const constexpr auto Minimum(const TNum1 & a, const TNumOthers & ... extras) -> decltype((a < Minimum(extras ...)) ? a : Minimum(extras ...))
{
    auto b = Minimum(extras ...);

    return (a < b) ? a : b;
}

template<typename TNum1, typename ... TNumOthers>
__bland __forceinline __const constexpr auto Minimum(const TNum1 & a, const TNumOthers ... extras) -> decltype((a < Minimum(extras ...)) ? a : Minimum(extras ...))
{
    auto b = Minimum(extras ...);

    return (a < b) ? a : b;
}//*/

template<typename TNum1>
__bland __forceinline __const constexpr TNum1 Maximum(const TNum1 & a)
{
    return a;
}
template<typename TNum1, typename TNum2>
__bland __forceinline __const constexpr auto Maximum(const TNum1 & a, const TNum2 & b) -> decltype((a > b) ? a : b)
{
    return (a > b) ? a : b;
}
template<typename TNum1, typename TNum2, typename TNum3>
__bland __forceinline __const constexpr auto Maximum(const TNum1 & a, const TNum2 & b, const TNum3 & c) -> decltype((a > Maximum(b, c)) ? a : Maximum(b, c))
{
    auto _b = Maximum(b, c);

    return (a > _b) ? a : _b;
}

#pragma GCC diagnostic pop

template<typename TNum>
__bland __forceinline __const constexpr TNum GreatestCommonDivisor(const TNum a)
{
    return a;
}

template<typename TNum>
__bland __forceinline __const constexpr TNum GreatestCommonDivisor(TNum a, TNum b)
{
    //  I hate homework.
    
    TNum t;

    while (b != 0)
    {
        t = b;
        b = a % b;
        a = t;
    }

    return a;
}

template<typename TNum>
__bland __forceinline __const constexpr TNum GreatestCommonDivisor(TNum a, TNum b, const TNum c)
{
    b = GreatestCommonDivisor(b, c);

    TNum t;

    while (b != 0)
    {
        t = b;
        b = a % b;
        a = t;
    }

    return a;
}

#else

#ifdef __BEELZEBUB__ARCH_X86
__bland __forceinline uint64_t RoundUp64(const uint64_t value, const uint64_t step) __const
{
    return ((value + step - 1) / step) * step;
}
__bland __forceinline uint32_t RoundUp32(const uint32_t value, const uint32_t step) __const
{
    return ((value + step - 1) / step) * step;
}
#else
__bland __forceinline uint64_t RoundUp64(const uint64_t value, const uint64_t step) __const
{
    return value + ((step - (value % step)) % step);
}
__bland __forceinline uint32_t RoundUp32(const uint32_t value, const uint32_t step) __const
{
    return value + ((step - (value % step)) % step);
}
#endif

__bland __forceinline uint64_t RoundDown64(const uint64_t value, const uint64_t step) __const
{
    return value - (value & step);
}
__bland __forceinline uint32_t RoundDown32(const uint32_t value, const uint32_t step) __const
{
    return value - (value & step);
}

__bland __forceinline uint64_t RoundUpDiff64(const uint64_t value, const uint64_t step) __const
{
    return (step - (value % step)) % step;
}
__bland __forceinline uint32_t RoundUpDiff32(const uint32_t value, const uint32_t step) __const
{
    return (step - (value % step)) % step;
}

 #define MIN(aP, bP)             \
   ({  __typeof__ (a) _a = (aP); \
       __typeof__ (b) _b = (bP); \
       _a < _b ? _a : _b;        })
 #define MAX(aP, bP)             \
   ({  __typeof__ (a) _a = (aP); \
       __typeof__ (b) _b = (bP); \
       _a > _b ? _a : _b;        })
//  Courtesy of http://stackoverflow.com/a/3437484/485098

__bland __forceinline uint64_t GreatestCommonDivisor64(uint64_t a, uint64_t b) __const
{
    uint64_t t;

    while (b != 0)
    {
        t = b;
        b = a % b;
        a = t;
    }

    return a;
}

__bland __forceinline uint64_t GreatestCommonDivisor32(uint32_t a, uint32_t b) __const
{
    uint32_t t;

    while (b != 0)
    {
        t = b;
        b = a % b;
        a = t;
    }

    return a;
}

#endif
