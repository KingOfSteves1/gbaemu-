#pragma once

#include <array>

#include "common/macros.h"
#include "common/utility.h"
#include "registers.h"

struct Memory;

class ARM : public Registers
{
public:
    void reset();

    int emulate();

    Memory* mem;

private:
    using InstructionArm = void(ARM::*)(u32);
    using InstructionThumb = void(ARM::*)(u16);

    using InstructionTableArm = std::array<InstructionArm, 4096>;
    using InstructionTableThumb = std::array<InstructionThumb, 1024>;

    enum class Access
    {
        Seq = 0,
        Nonseq = 1,
    };

    enum class Shift
    {
        LSL = 0b00,
        LSR = 0b01,
        ASR = 0b10,
        ROR = 0b11
    };

    enum class State
    {
        Arm = 0,
        Thumb = 1
    };

    u8  readByte(u32 addr);
    u16 readHalf(u32 addr);
    u32 readWord(u32 addr);

    void writeByte(u32 addr, u8  byte);
    void writeHalf(u32 addr, u16 half);
    void writeWord(u32 addr, u32 word);

    u32 readWordRotated(u32 addr);
    u32 readHalfRotated(u32 addr);
    u32 readHalfSigned(u32 addr);

    u32 lsl(u32 value, int amount, bool& carry) const;
    u32 lsr(u32 value, int amount, bool& carry, bool immediate) const;
    u32 asr(u32 value, int amount, bool& carry, bool immediate) const;
    u32 ror(u32 value, int amount, bool& carry, bool immediate) const;
    u32 shift(Shift type, u32 value, int amount, bool& carry, bool immediate) const;

    u32 lsl(u32 value, int amount) const;
    u32 lsr(u32 value, int amount, bool immediate) const;
    u32 asr(u32 value, int amount, bool immediate) const;
    u32 ror(u32 value, int amount, bool immediate) const;
    u32 shift(Shift type, u32 value, int amount, bool immediate) const;

    inline u32 logical(u32 result, bool flags);
    inline u32 logical(u32 result, bool carry, bool flags);

    inline u32 add(u32 op1, u32 op2, bool flags);
    inline u32 sub(u32 op1, u32 op2, bool flags);

    template<State state>
    inline void advance();
    inline void advance();
    template<State state>
    inline void refill();

    void HWI();
    void SWI();
    void interrupt(u32 pc, u32 lr, PSR::Mode mode);

    void dispatch();
    void debug();

    template<Access access>
    inline void cycle(u32 addr);
    inline void cycle();
    inline void cycleBooth(u32 multiplier, bool allow_ones);

    #include "isa-arm.inl"
    #include "isa-thumb.inl"

    u64 cycles;

    static InstructionTableArm instr_arm;
    static InstructionTableThumb instr_thumb;
};

#include "arm.inl"
