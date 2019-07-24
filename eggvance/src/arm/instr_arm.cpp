#include "arm.h"

#include "common/format.h"
#include "common/utility.h"
#include "utility.h"

u32 ARM::shiftedRegister(int data, bool& carry)
{
    int rm      = bits<0, 4>(data);
    int use_reg = bits<4, 1>(data);
    int type    = bits<5, 2>(data);

    int offset;
    if (use_reg)
    {
        int rs = bits<8, 4>(data);
        offset = regs[rs];
        offset &= 0xFF;
    }
    else
    {
        offset = bits<7, 5>(data);
    }

    u32 value = regs[rm];
    switch (type)
    {
    case 0b00: return lsl(value, offset, carry); break;
    case 0b01: return lsr(value, offset, carry, !use_reg); break;
    case 0b10: return asr(value, offset, carry, !use_reg); break;
    case 0b11: return ror(value, offset, carry, !use_reg); break;
    }
    return value;
}

u32 ARM::rotatedImmediate(int data, bool& carry)
{
    u32 value    = bits<0, 8>(data);
    int rotation = bits<8, 4>(data);

    return ror(value, rotation << 1, carry, false);
}

void ARM::branchExchange(u32 instr)
{
    int rn = bits<0, 4>(instr);

    u32 addr = regs[rn];
    if (addr & 0x1)
    {
        addr = alignHalf(addr);
        regs.thumb = true;
    }
    else
    {
        addr = alignWord(addr);
    }

    cycle(regs.pc, NSEQ);

    regs.pc = addr;
    advance();

    cycle(regs.pc, SEQ);
    cycle(regs.pc + (regs.thumb ? 2 : 4), SEQ);
}

void ARM::branchLink(u32 instr)
{
    int offset = bits< 0, 24>(instr);
    int link   = bits<24,  1>(instr);

    offset = signExtend<24>(offset);
    offset <<= 2;

    if (link) 
        regs.lr = regs.pc - 4;

    cycle(regs.pc, NSEQ);

    regs.pc += offset;
    advance();

    cycle(regs.pc, SEQ);
    cycle(regs.pc + 4, SEQ);
}

void ARM::dataProcessing(u32 instr)
{
    int data    = bits< 0, 12>(instr);
    int rd      = bits<12,  4>(instr);
    int rn      = bits<16,  4>(instr);
    int flags   = bits<20,  1>(instr);
    int opcode  = bits<21,  4>(instr);
    int use_imm = bits<25,  1>(instr);

    u32& dst = regs[rd];
    u32  op1 = regs[rn];
    u32  op2;

    bool carry;
    if (use_imm)
    {
        op2 = rotatedImmediate(data, carry);
    }
    else
    {
        int rm      = bits<0, 4>(data);
        int use_reg = bits<4, 1>(data);
        int type    = bits<5, 2>(data);

        u32 value = regs[rm];

        int offset;
        if (use_reg)
        {
            int rs = bits<8, 4>(instr);
            offset = regs[rs];
            offset &= 0xFF;

            // Account for prefetch
            if (rn == 15) op1 += 4;
            if (rm == 15) value += 4;
        }
        else
        {
            offset = bits<7, 5>(data);
        }

        switch (type)
        {
        case 0b00: op2 = lsl(value, offset, carry); break;
        case 0b01: op2 = lsr(value, offset, carry, !use_reg); break;
        case 0b10: op2 = asr(value, offset, carry, !use_reg); break;
        case 0b11: op2 = ror(value, offset, carry, !use_reg); break;
        }
        cycle();
    }

    if (rd == 15)
    {
        cycle(regs.pc, NSEQ);

        if (flags)
        {
            u32 spsr = regs.spsr;
            regs.switchMode(static_cast<Mode>(spsr & CPSR_M));
            regs.cpsr = spsr;

            flags = false;
        }
    }

    switch (opcode)
    {
    // AND
    case 0b0000:
        dst = op1 & op2;
        if (flags) 
            logical(dst, carry);
        break;

    // EOR
    case 0b0001:
        dst = op1 ^ op2;
        if (flags) 
            logical(dst, carry);
        break;

    // SUB
    case 0b0010:
        dst = op1 - op2;
        if (flags) 
            arithmetic(op1, op2, false);
        break;

    // RSB
    case 0b0011:
        dst = op2 - op1;
        if (flags) 
            arithmetic(op2, op1, false);
        break;

    // ADD
    case 0b0100:
        dst = op1 + op2;
        if (flags) 
            arithmetic(op1, op2, true);
        break;

    // ADC
    case 0b0101:
        op2 += regs.c;
        dst = op1 + op2;
        if (flags) 
            arithmetic(op1, op2, true);
        break;

    // SBC
    case 0b0110:
        op2 += 1 - regs.c;
        dst = op1 - op2;
        if (flags) 
            arithmetic(op1, op2, false);
        break;

    // RBC
    case 0b0111:
        op1 += 1 - regs.c;
        dst = op2 - op1;
        if (flags) 
            arithmetic(op2, op1, false);
        break;

    // TST
    case 0b1000:
        logical(op1 & op2, carry);
        break;

    // TEQ
    case 0b1001:
        logical(op1 ^ op2, carry);
        break;

    // CMP
    case 0b1010:
        arithmetic(op1, op2, false);
        break;

    // CMN
    case 0b1011:
        arithmetic(op1, op2, true);
        break;

    // ORR
    case 0b1100:
        dst = op1 | op2;
        if (flags) 
            logical(dst, carry);
        break;

    // MOV
    case 0b1101:
        dst = op2;
        if (flags) 
            logical(dst, carry);
        break;

    // BIC
    case 0b1110:
        dst = op1 & ~op2;
        if (flags) 
            logical(dst, carry);
        break;

    // MVN
    case 0b1111:
        dst = ~op2;
        if (flags) 
            logical(dst, carry);
        break;
    }

    if (rd == 15)
    {
        // Interrupt return from ARM into THUMB possible
        dst = regs.thumb ? alignHalf(dst) : alignWord(dst);
        advance();

        cycle(regs.pc, SEQ);
    }
    cycle(regs.pc + 4, SEQ);
}

void ARM::psrTransfer(u32 instr)
{
    int write    = bits<21, 1>(instr);
    int use_spsr = bits<22, 1>(instr);

    if (write)
    {
        int data    = bits< 0, 12>(instr);
        int use_imm = bits<25,  1>(instr);

        u32 op;
        if (use_imm)
        {
            bool carry;
            op = rotatedImmediate(data, carry);
        }
        else
        {
            int rm = bits<0, 4>(data);
            op = regs[rm];
        }

        // Mask based on fsxc-bits
        u32 mask = 0;
        if (instr & (1 << 19)) mask |= 0xFF000000;
        if (instr & (1 << 18)) mask |= 0x00FF0000;
        if (instr & (1 << 17)) mask |= 0x0000FF00;
        if (instr & (1 << 16))
        {
            // Control bits are protected in USR mode
            if (regs.mode() != MODE_USR)
                mask |= 0x000000FF;
        }

        op &= mask;

        if (use_spsr)
        {
            regs.spsr = (regs.spsr & ~mask) | op;
        }
        else
        {
            if (mask & 0xFF)
                regs.switchMode(static_cast<Mode>(op & CPSR_M));

            regs.cpsr = (regs.cpsr & ~mask) | op;

            if (regs.thumb)
            {
                // Undefined behavior
                regs.pc = alignHalf(regs.pc);
                advance();
            }
        }
    }
    else
    {
        int rd = bits<12, 4>(instr);
        regs[rd] = use_spsr ? regs.spsr : regs.cpsr;
    }
    cycle(regs.pc + 4, SEQ);
}

void ARM::multiply(u32 instr)
{
    int rm         = bits< 0, 4>(instr);
    int rs         = bits< 8, 4>(instr);
    int rn         = bits<12, 4>(instr);
    int rd         = bits<16, 4>(instr);
    int flags      = bits<20, 1>(instr);
    int accumulate = bits<21, 1>(instr);

    u32& dst = regs[rd];
    u32  op1 = regs[rs];
    u32  op2 = regs[rm];
    u32  acc = regs[rn];

    dst = op1 * op2;
    if (accumulate)
    {
        dst += acc;
        cycle();
    }

    if (flags)
        logical(dst);

    cycleBooth(op1, true);
    cycle(regs.pc + 4, SEQ);
}

void ARM::multiplyLong(u32 instr)
{
    int rm         = bits< 0, 4>(instr);
    int rs         = bits< 8, 4>(instr);
    int rdlo       = bits<12, 4>(instr);
    int rdhi       = bits<16, 4>(instr);
    int flags      = bits<20, 1>(instr);
    int accumulate = bits<21, 1>(instr);
    int sign       = bits<22, 1>(instr);

    u32& dsthi = regs[rdhi];
    u32& dstlo = regs[rdlo];

    u64 op1 = regs[rs];
    u64 op2 = regs[rm];

    if (sign)
    {
        op1 = signExtend<32>(op1);
        op2 = signExtend<32>(op2);
    }
    
    u64 result = op1 * op2;
    if (accumulate)
    {
        result += (static_cast<u64>(dsthi) << 32) | dstlo;
        cycle();
    }

    dstlo = static_cast<u32>(result);
    dsthi = static_cast<u32>(result >> 32);

    if (flags)
    {
        regs.z = result == 0;
        regs.n = result >> 63;
    }

    cycle();
    cycleBooth(static_cast<u32>(op1), sign);
    cycle(regs.pc + 4, SEQ);
}

void ARM::singleDataTransfer(u32 instr)
{
    int data      = bits< 0, 12>(instr);
    int rd        = bits<12,  4>(instr);
    int rn        = bits<16,  4>(instr);
    int load      = bits<20,  1>(instr);
    int writeback = bits<21,  1>(instr);
    int byte      = bits<22,  1>(instr);
    int increment = bits<23,  1>(instr);
    int pre_index = bits<24,  1>(instr);
    int use_reg   = bits<25,  1>(instr);

    u32& dst = regs[rd];
    u32 addr = regs[rn];

    // Post-indexing forces writeback
    writeback |= !pre_index;

    u32 offset;
    if (use_reg)
    {
        bool carry;
        offset = shiftedRegister(data, carry);
    }
    else
    {
        offset = data;
    }

    if (pre_index)
    {
        if (increment)
            addr += offset;
        else
            addr -= offset;
    }

    cycle(regs.pc, NSEQ);

    if (load)
    {
        cycle();
        if (rd == 15)
            cycle(regs.pc + 4, NSEQ);

        if (byte)
            dst = mmu.readByte(addr);
        else
            dst = ldr(addr);

        // Prevent overwriting loaded value
        if (rd == rn)
            writeback = false;

        if (rd == 15)
        {
            dst = alignWord(dst);
            advance();

            cycle(regs.pc, SEQ);
        }
        cycle(regs.pc + 4, SEQ);
    }
    else
    {
        u32 value = dst;
        // Account for prefetch
        if (rd == 15) value += 4;

        if (byte)
            mmu.writeByte(addr, value);
        else
            mmu.writeWord(alignWord(addr), value);

        cycle(addr, NSEQ);
    }

    if (!pre_index)
    {
        if (increment)
            addr += offset;
        else
            addr -= offset;
    }
    
    if (writeback)
        regs[rn] = addr;
}

void ARM::halfwordSignedDataTransfer(u32 instr)
{
    int half      = bits< 5, 1>(instr);
    int sign      = bits< 6, 1>(instr);
    int rd        = bits<12, 4>(instr);
    int rn        = bits<16, 4>(instr);
    int load      = bits<20, 1>(instr);
    int writeback = bits<21, 1>(instr);
    int use_imm   = bits<22, 1>(instr);
    int increment = bits<23, 1>(instr);
    int pre_index = bits<24, 1>(instr);

    u32 addr = regs[rn];
    u32& dst = regs[rd];

    // Post-indexing forces writeback
    writeback |= !pre_index;

    u32 offset;
    if (use_imm)
    {
        int lower = bits<0, 4>(instr);
        int upper = bits<8, 4>(instr);
        offset = (upper << 4) | lower;
    }
    else
    {
        int rm = bits<0, 4>(instr);
        offset = regs[rm];
    }

    if (pre_index)
    {
        if (increment)
            addr += offset;
        else
            addr -= offset;
    }

    cycle(regs.pc, NSEQ);

    if (load)
    {
        cycle();
        if (rd == 15)
            cycle(regs.pc + 4, NSEQ);

        switch ((sign << 1) | half)
        {
        // LDRH
        case 0b01:
            dst = ldrh(addr);
            break;

        // LDRSB
        case 0b10:
            dst = mmu.readByte(addr);
            dst = signExtend<8>(dst);
            break;

        // LDRSH
        case 0b11:
            dst = ldrsh(addr);
            break;
        }

        // Prevent overwriting loaded value
        if (rd == rn)
            writeback = false;

        if (rd == 15)
        {
            dst = alignWord(dst);
            advance();

            cycle(regs.pc, SEQ);
        }
        cycle(regs.pc + 4, SEQ);
    }
    else  // STRH
    {
        u32 value = dst;
        // Account for prefetch
        if (rd == 15) value += 4;

        mmu.writeHalf(alignHalf(addr), value);

        cycle(addr, NSEQ);
    }

    if (!pre_index)
    {
        if (increment)
            addr += offset;
        else
            addr -= offset;
    }

    if (writeback)
        regs[rn] = addr;
}

void ARM::blockDataTransfer(u32 instr)
{
    int rlist     = bits< 0, 16>(instr);
    int rn        = bits<16,  4>(instr);
    int load      = bits<20,  1>(instr);
    int writeback = bits<21,  1>(instr);
    int user      = bits<22,  1>(instr);
    int ascending = bits<23,  1>(instr);
    int full      = bits<24,  1>(instr);

    u32 addr = regs[rn];

    // Force user register transfer
    Mode mode = regs.mode();
    if (user)
        regs.switchMode(MODE_USR);

    if (rlist != 0)
    {
        cycle(regs.pc, NSEQ);

        // Register count needed for cycles
        int rcount = count_bits(rlist) + count_bits(rlist >> 8);

        int init = ascending ? 0 : 15;
        int loop = ascending ? 1 : -1;
        int step = ascending ? 4 : -4;

        // Lowest register is stored at the lowest address
        if (load)
        {
            // Prevent overwriting loaded value
            if (rlist & (1 << rn))
                writeback = false;

            for (int x = init; rcount > 0; x += loop)
            {
                if (rlist & (1 << x))
                {
                    if (full) addr += step;

                    if (--rcount > 0)
                        cycle(addr, SEQ);
                    else
                        cycle();

                    if (x == 15)
                        cycle(regs.pc + 4, NSEQ);

                    regs[x] = mmu.readWord(alignWord(addr));

                    if (x == 15)
                    {
                        regs.pc = alignWord(regs.pc);
                        advance();

                        cycle(regs.pc, SEQ);
                    }

                    if (!full) addr += step;
                }
            }
            cycle(regs.pc + 4, SEQ);
        }
        else
        {
            for (int x = init; rcount > 0; x += loop)
            {
                if (rlist & (1 << x))
                {
                    if (full) addr += step;

                    if (--rcount > 0)
                        cycle(addr, SEQ);

                    mmu.writeWord(alignWord(addr), regs[x]);

                    if (!full) addr += step;
                }
            }
            cycle(addr, NSEQ);
        }
    }
    else  // Special case empty rlist
    {
        if (load)
        {
            regs.pc = mmu.readWord(alignWord(addr));
            regs.pc = alignWord(regs.pc);
            advance();
        }
        else
        {
            // Save address of next instruction
            mmu.writeWord(alignWord(addr), regs.pc + 4);
        }
        addr += ascending ? 0x40 : -0x40;
    }

    if (writeback)
        regs[rn] = addr;

    if (user)
        regs.switchMode(mode);
}

void ARM::singleDataSwap(u32 instr)
{
    int rm   = bits< 0, 4>(instr);
    int rd   = bits<12, 4>(instr);
    int rn   = bits<16, 4>(instr);
    int byte = bits<22, 1>(instr);

    u32 addr = regs[rn];
    u32& dst = regs[rd];
    u32  src = regs[rm];

    cycle(regs.pc, NSEQ);
    cycle(addr, NSEQ);

    if (byte)
    {
        dst = mmu.readByte(addr);
        mmu.writeByte(addr, src);
    }
    else
    {
        dst = ldr(addr);
        mmu.writeWord(alignWord(addr), src);
    }

    cycle();
    cycle(regs.pc + 4, SEQ);
}

void ARM::softwareInterruptArm(u32 instr)
{
    cycle(regs.pc, NSEQ);

    u32 cpsr = regs.cpsr;
    u32 next = regs.pc - 4;

    regs.switchMode(MODE_SVC);
    regs.spsr = cpsr;
    regs.lr = next;

    regs.irqd = true;

    regs.pc = EXV_SWI;
    advance();

    cycle(regs.pc, SEQ);
    cycle(regs.pc + 4, SEQ);
}