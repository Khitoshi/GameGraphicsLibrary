#pragma once

#include "CommonHeaders.h"
#include "PrimitiveTypes.h"
#include "MathTypes.h"

namespace Math {

    constexpr bool IsEqual(f32 a, f32 b, f32 eps = epsilon)
    {
        f32 diff{ a - b };
        if (diff < 0.f) diff = -diff;
        return diff < eps;
    }

    template<typename T>
    constexpr T Clamp(T value, T min, T max)
    {
        return (value < min) ? min : (value > max) ? max : value;
    }

    template<u32 bits>
    constexpr u32 PackUnitFloat(f32 f)
    {
        static_assert(bits <= sizeof(u32) * 8);
        assert(f >= 0.f && f <= 1.f);
        constexpr f32 intervals{ (f32)((1ui32 << bits) - 1) };
        return (u32)(intervals * f + 0.5f);
    }

    template<u32 bits>
    constexpr f32 UnpackToUnitFloat(u32 i)
    {
        static_assert(bits <= sizeof(u32) * 8);
        assert(i < (1ui32 << bits));
        constexpr f32 intervals{ (f32)((1ui32 << bits) - 1) };
        return (f32)i / intervals;
    }

    template<u32 bits>
    constexpr u32 PackFloat(f32 f, f32 min, f32 max)
    {
        assert(min < max);
        assert(f <= max && f >= min);
        const f32 distance{ (f - min) / (max - min) };
        return PackUnitFloat<bits>(distance);
    }

    template<u32 bits>
    constexpr f32 UnpackToFloat(u32 i, f32 min, f32 max)
    {
        assert(min < max);
        return unpack_to_unit_float<bits>(i) * (max - min) + min;
    }

    // lÌŒÜ“ü‚µ‚Ä‘µ‚¦‚éB®—ñ‚Ì”{”‚È‚é
    template<u64 alignment>
    constexpr u64 AlignSizeUp(u64 size)
    {
        static_assert(alignment, "Alignment must be non-zero.");
        constexpr u64 mask{ alignment - 1 };
        static_assert(!(alignment & mask), "Alignment should be a power of 2.");
        return ((size + mask) & ~mask);
    }

    // Ø‚èÌ‚Ä‚É‚æ‚Á‚Ä®—ñ‚·‚éB®—ñ‚Ì”{”‚È‚é
    template<u64 alignment>
    constexpr u64 AlignSizeDown(u64 size)
    {
        static_assert(alignment, "Alignment must be non-zero.");
        constexpr u64 mask{ alignment - 1 };
        static_assert(!(alignment & mask), "Alignment should be a power of 2.");
        return (size & ~mask);
    }
}
