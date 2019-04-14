#pragma once

#include <iomanip>
#include <sstream>

#include "integer.h"

template<typename T>
inline std::string hex(const T& value)
{
    static_assert(std::is_convertible<T, int>::value, "Type must be convertable to int");

    std::stringstream stream;

    int intValue = static_cast<int>(value);
    if (intValue < 0)
    {
        intValue = abs(intValue);
        stream << "-";
    }

    stream << "0x" << std::setw(2 * sizeof(T)) << std::setfill('0')
        << std::uppercase << std::hex << intValue;

    return stream.str();
}

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
    static_assert(bits <= 32, "Invalid number of bits");

    if (value & (1 << (bits - 1)))
    {
        u32 mask = (1 << bits) - 1;
        return -static_cast<s32>((~value + 1) & mask);
    }
    return value;
}

template<>
inline s32 twos<32>(u32 value)
{
    return static_cast<s32>(value);
}

template<unsigned int bits>
inline s32 twos2(u32 value)
{
    static_assert(bits <= 32, "Unsupported number of bits");

    if (value & (1 << (bits - 1)))
        value |= (0xFFFFFFFF << bits);

    return static_cast<s32>(value);
}

template<>
inline s32 twos2<32>(u32 value)
{
    return static_cast<s32>(value);
}
