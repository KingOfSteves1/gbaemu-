#pragma once

enum Page
{
    PAGE_BIOS         = 0x0,
    PAGE_WRAM         = 0x2,
    PAGE_IWRAM        = 0x3,
    PAGE_IO           = 0x4,
    PAGE_PALETTE      = 0x5,
    PAGE_VRAM         = 0x6,
    PAGE_OAM          = 0x7,
    PAGE_GAMEPAK_0    = 0x8,
    PAGE_GAMEPAK_1    = 0xA,
    PAGE_GAMEPAK_2    = 0xC,
    PAGE_GAMEPAK_SRAM = 0xE
};

enum Map
{
    MAP_BIOS         = 0x0000'0000,
    MAP_WRAM         = 0x0200'0000,
    MAP_IWRAM        = 0x0300'0000,
    MAP_IO           = 0x0400'0000,
    MAP_PALETTE      = 0x0500'0000,
    MAP_VRAM         = 0x0600'0000,
    MAP_OAM          = 0x0700'0000,
    MAP_GAMEPAK_0    = 0x0800'0000,
    MAP_GAMEPAK_1    = 0x0A00'0000,
    MAP_GAMEPAK_2    = 0x0C00'0000,
    MAP_GAMEPAK_SRAM = 0x0E00'0000
};

enum Register
{
    REG_DISPCNT  = 0x000,
    REG_DISPSTAT = 0x004,
    REG_VCOUNT   = 0x006,
    REG_BG0CNT   = 0x008,
    REG_BG1CNT   = 0x00A,
    REG_BG2CNT   = 0x00C,
    REG_BG3CNT   = 0x00E,
    REG_BG0HOFS  = 0x010,
    REG_BG0VOFS  = 0x012,
    REG_BG1HOFS  = 0x014,
    REG_BG1VOFS  = 0x016,
    REG_BG2HOFS  = 0x018,
    REG_BG2VOFS  = 0x01A,
    REG_BG3HOFS  = 0x01C,
    REG_BG3VOFS  = 0x01E,
    REG_BG2PA    = 0x020,
    REG_BG2PB    = 0x022,
    REG_BG2PC    = 0x024,
    REG_BG2PD    = 0x026,
    REG_BG2X     = 0x028,
    REG_BG2Y     = 0x02C,
    REG_BG3PA    = 0x030,
    REG_BG3PB    = 0x032,
    REG_BG3PC    = 0x034,
    REG_BG3PD    = 0x036,
    REG_BG3X     = 0x038,
    REG_BG3Y     = 0x03C,
    REG_WIN0H    = 0x040,
    REG_WIN1H    = 0x042,
    REG_WIN0V    = 0x044,
    REG_WIN1V    = 0x046,
    REG_WININ    = 0x048,
    REG_WINOUT   = 0x04A,
    REG_MOSAIC   = 0x04C,
    REG_BLDCNT   = 0x050,
    REG_BLDALPHA = 0x052,
    REG_BLDY     = 0x054,
    REG_TM0CNT_L = 0x100,
    REG_TM0CNT_H = 0x102,
    REG_TM1CNT_L = 0x104,
    REG_TM1CNT_H = 0x106,
    REG_TM2CNT_L = 0x108,
    REG_TM2CNT_H = 0x10A,
    REG_TM3CNT_L = 0x10C,
    REG_TM3CNT_H = 0x10E,
    REG_KEYINPUT = 0x130,
    REG_KEYCNT   = 0x132,
    REG_IE       = 0x200,
    REG_IF       = 0x202,
    REG_WAITCNT  = 0x204,
    REG_IME      = 0x208,
    REG_HALTCNT  = 0x301
};
