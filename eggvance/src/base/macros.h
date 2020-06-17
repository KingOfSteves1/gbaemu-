#pragma once

#include <cassert>

#define ASSERT(cond, msg) assert((cond) && msg)

#ifdef NDEBUG
#  ifdef _MSC_VER
#  define UNREACHABLE __assume(0)
#  else
#  define UNREACHABLE __builtin_unreachable()
#  endif
#else
#  define UNREACHABLE ASSERT(false, "Unreachable")
#endif

#define CASE_BYTE_REG(label)                                            \
    case label + 0

#define CASE_HALF_REG(label)                                            \
    case label + 0:                                                     \
    case label + 1

#define CASE_WORD_REG(label)                                            \
    case label + 0:                                                     \
    case label + 1:                                                     \
    case label + 2:                                                     \
    case label + 3

#define READ_BYTE_REG(label, reg)                                       \
    case label + 0: return reg.read<0>()

#define READ_HALF_REG(label, reg)                                       \
    case label + 0: return reg.read<0>();                               \
    case label + 1: return reg.read<1>()

#define READ_WORD_REG(label, reg)                                       \
    case label + 0: return reg.read<0>();                               \
    case label + 1: return reg.read<1>();                               \
    case label + 2: return reg.read<2>();                               \
    case label + 3: return reg.read<3>()

#define WRITE_BYTE_REG(label, reg, mask)                                \
    case label + 0: reg.write<0>(byte & ((mask >>  0) & 0xFF)); break

#define WRITE_HALF_REG(label, reg, mask)                                \
    case label + 0: reg.write<0>(byte & ((mask >>  0) & 0xFF)); break;  \
    case label + 1: reg.write<1>(byte & ((mask >>  8) & 0xFF)); break

#define WRITE_WORD_REG(label, reg, mask)                                \
    case label + 0: reg.write<0>(byte & ((mask >>  0) & 0xFF)); break;  \
    case label + 1: reg.write<1>(byte & ((mask >>  8) & 0xFF)); break;  \
    case label + 2: reg.write<2>(byte & ((mask >> 16) & 0xFF)); break;  \
    case label + 3: reg.write<3>(byte & ((mask >> 24) & 0xFF)); break

#define WRITE_BYTE_REGX(label, reg)                                     \
    case label + 0: reg.write<0>(byte); break

#define WRITE_HALF_REGX(label, reg)                                     \
    case label + 0: reg.write<0>(byte); break;                          \
    case label + 1: reg.write<1>(byte); break

#define WRITE_WORD_REGX(label, reg)                                     \
    case label + 0: reg.write<0>(byte); break;                          \
    case label + 1: reg.write<1>(byte); break;                          \
    case label + 2: reg.write<2>(byte); break;                          \
    case label + 3: reg.write<3>(byte); break

#define CASE1(label, expr)                                                                                  \
    case label + 0: { constexpr uint kIndex = 0; constexpr uint kLabel = label + kIndex; expr; break; }

#define CASE2(label, expr)                                                                                  \
    case label + 0: { constexpr uint kIndex = 0; constexpr uint kLabel = label + kIndex; expr; break; }     \
    case label + 1: { constexpr uint kIndex = 1; constexpr uint kLabel = label + kIndex; expr; break; }

#define CASE4(label, expr)                                                                                  \
    case label + 0: { constexpr uint kIndex = 0; constexpr uint kLabel = label + kIndex; expr; break; }     \
    case label + 1: { constexpr uint kIndex = 1; constexpr uint kLabel = label + kIndex; expr; break; }     \
    case label + 2: { constexpr uint kIndex = 2; constexpr uint kLabel = label + kIndex; expr; break; }     \
    case label + 3: { constexpr uint kIndex = 3; constexpr uint kLabel = label + kIndex; expr; break; }
