#pragma once

#include "common/integer.h"

inline u32 alignHalf(u32 addr)
{
    return addr & ~0x1;
}

inline u32 alignWord(u32 addr)
{
    return addr & ~0x3;
}

inline bool misalignedHalf(u32 addr)
{
    return addr & 0x1;
}

inline bool misalignedWord(u32 addr)
{
    return addr & 0x3;
}

template<unsigned int bits>
inline s32 twos(u32 value)
{
    static_assert(bits <= 32, "Unsupported number of bits");

    if (value & (1 << (bits - 1)))
        value |= 0xFFFFFFFF << bits;

    return static_cast<s32>(value);
}

template<>
inline s32 twos<32>(u32 value)
{
    return static_cast<s32>(value);
}