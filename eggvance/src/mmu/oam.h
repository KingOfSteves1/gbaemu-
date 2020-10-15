#pragma once

#include <array>

#include "mmu/ram.h"
#include "ppu/oamentry.h"
#include "ppu/matrix.h"

class Oam : public Ram<0x400>
{
public:
    void writeByte(u32 addr, u8  byte) = delete;
    void writeHalf(u32 addr, u16 half);
    void writeWord(u32 addr, u32 word);

    Matrix matrix(uint index) const;

    std::array<OAMEntry, 128> entries;
};
