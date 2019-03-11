#include "registers.h"

#include "common/log.h"

void Registers::reset()
{
    *bank = { };
    *bank_fiq = { };
    *bank_spsr = { };

    pc = 0x8000000;
    
    sp = 0x03007F00;
    bank[BANK_FIQ].sp = 0x03007F00;
    bank[BANK_ABT].sp = 0x03007F00;
    bank[BANK_UND].sp = 0x03007F00;
    bank[BANK_SVC].sp = 0x03007FE0;
    bank[BANK_IRQ].sp = 0x03007FA0;

    cpsr = 0x5F;
    spsr = nullptr;

    // Setup values for test ROMs
    cpsr |= CPSR_T;
    pc = 0x080000C8;
}

bool Registers::isThumb() const
{
    return cpsr & CPSR_T;
}

u8 Registers::z() const
{
    return (cpsr & CPSR_Z) ? 1 : 0;
}

u8 Registers::n() const
{
    return (cpsr & CPSR_N) ? 1 : 0;
}

u8 Registers::c() const
{
    return (cpsr & CPSR_C) ? 1 : 0;
}

u8 Registers::v() const
{
    return (cpsr & CPSR_V) ? 1 : 0;
}

void Registers::setZ(bool set)
{
    setFlag(CPSR_Z, set);
}

void Registers::setN(bool set)
{
    setFlag(CPSR_N, set);
}

void Registers::setC(bool set)
{
    setFlag(CPSR_C, set);
}

void Registers::setV(bool set)
{
    setFlag(CPSR_V, set);
}

bool Registers::checkCondition(Condition cond) const
{
    u8 z = this->z();
    u8 n = this->n();
    u8 c = this->c();
    u8 v = this->v();

    switch (cond)
    {
    case COND_EQ: return z;
    case COND_NE: return !z;
    case COND_CS: return c;
    case COND_CC: return !c;
    case COND_MI: return n;
    case COND_PL: return !n;
    case COND_VS: return v;
    case COND_VC: return !v;
    case COND_HI: return c && !z;
    case COND_LS: return !c || z;
    case COND_GE: return n == v;
    case COND_LT: return n != v;
    case COND_GT: return !z && (n == v);
    case COND_LE: return z || (n != v);
    case COND_AL: return true;
    case COND_NV: return false;

    default:
        log() << "Invalid condition " << (int)cond;
    }
    return true;
}

u32 Registers::operator[](int index) const
{
    return regs[index];
}

u32& Registers::operator[](int index)
{
    return regs[index];
}

void Registers::setFlag(CPSR flag, bool set)
{
    if (set)
        cpsr |= flag;
    else
        cpsr &= ~flag;
}
