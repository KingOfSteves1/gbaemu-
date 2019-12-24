#pragma once

#include "common/bitutil.h"
#include "common/integer.h"
#include "common/macros.h"

struct PSR
{
    enum class Mode
    {
        USR = 0b10000,
        FIQ = 0b10001,
        IRQ = 0b10010,
        SVC = 0b10011,
        ABT = 0b10111,
        SYS = 0b11111,
        UND = 0b11011
    };

    enum class Condition
    {
        EQ = 0x0,
        NE = 0x1,
        CS = 0x2,
        CC = 0x3,
        MI = 0x4,
        PL = 0x5,
        VS = 0x6,
        VC = 0x7,
        HI = 0x8,
        LS = 0x9,
        GE = 0xA,
        LT = 0xB,
        GT = 0xC,
        LE = 0xD,
        AL = 0xE,
        NV = 0xF
    };

    inline PSR& operator=(u32 value)
    {
        mode = static_cast<Mode>(bitutil::get<0, 5>(value));
        t    = bitutil::get< 5, 1>(value);
        f    = bitutil::get< 6, 1>(value);
        i    = bitutil::get< 7, 1>(value);
        v    = bitutil::get<28, 1>(value);
        c    = bitutil::get<29, 1>(value);
        z    = bitutil::get<30, 1>(value);
        n    = bitutil::get<31, 1>(value);

        return *this;
    }

    inline operator u32() const
    {
        return static_cast<int>(mode)
            | (t <<  5)
            | (f <<  6)
            | (i <<  7)
            | (v << 28)
            | (c << 29)
            | (z << 30)
            | (n << 31);
    }

    inline bool check(Condition condition) const
    {
        switch (condition)
        {
        case Condition::EQ: return z;
        case Condition::NE: return !z;
        case Condition::CS: return c;
        case Condition::CC: return !c;
        case Condition::MI: return n;
        case Condition::PL: return !n;
        case Condition::VS: return v;
        case Condition::VC: return !v;
        case Condition::HI: return c && !z;
        case Condition::LS: return !c || z;
        case Condition::GE: return n == v;
        case Condition::LT: return n != v;
        case Condition::GT: return !z && (n == v);
        case Condition::LE: return z || (n != v);
        case Condition::AL: return true;
        case Condition::NV: return false;

        default:
            EGG_UNREACHABLE;
            return false;
        }
    }

    inline int instrSize() const
    {
        return 2 * (t ^ 0x1) + 2;
    }

    Mode mode;
    int t;
    int f;
    int i;
    int v;
    int c;
    int z;
    int n;
};
