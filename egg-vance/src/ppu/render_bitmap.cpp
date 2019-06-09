#include "ppu.h"

#include "mmu/map.h"

void PPU::renderMode3()
{
    if (!mmu.dispcnt.bg2)
        return;

    u32 addr = mmu.dispcnt.frameAddr();

    int y = mmu.vcount.line;
    for (int x = 0; x < WIDTH; ++x)
    {
        int offset = 2 * (WIDTH * y + x);
        int color = mmu.readHalfFast(addr + offset);

        buffer_bg2[x] = color;
    }
}

void PPU::renderMode4()
{
    if (!mmu.dispcnt.bg2)
        return;

    u32 addr = mmu.dispcnt.frameAddr();

    int y = mmu.vcount.line;
    for (int x = 0; x < WIDTH; ++x)
    {
        int offset = WIDTH * y + x;
        int index = mmu.readByteFast(addr + offset);
        int color = readBgColor(index, 0);

        buffer_bg2[x] = color;
    }
}

void PPU::renderMode5()
{
    if (!mmu.dispcnt.bg2)
        return;

    u32 addr = mmu.dispcnt.frameAddr();

    int y = mmu.vcount.line;
    for (int x = 0; x < WIDTH; ++x)
    {
        int color = COLOR_TRANSPARENT;
        if (x < 160 && y < 128)
        {
            int offset = 2 * (160 * y + x);
            color = mmu.readHalfFast(addr + offset);
        }
        buffer_bg2[x] = color;
    }
}
