#pragma once

#include "gamepak/save.h"

class Flash : public Save
{
public:
    Flash(const fs::path& file, uint size);

    u8 read(u32 addr) final;
    void write(u32 addr, u8 byte) final;

private:
    enum Command
    {
        CMD_ERASE         = 0xAA5580,
        CMD_ERASE_CHIP    = 0xAA5510,
        CMD_ERASE_SECTOR  = 0xAA5530,
        CMD_WRITE_BYTE    = 0xAA55A0,
        CMD_SWITCH_BANK   = 0xAA55B0,
        CMD_CHIP_ID_ENTER = 0xAA5590,
        CMD_CHIP_ID_EXIT  = 0xAA55F0
    };

    bool id;
    bool erase;
    u32 command;
    u8* bank;
};

class Flash64 : public Flash
{
public:
    Flash64(const fs::path& file)
        : Flash(file, 0x10'000) {}
};

class Flash128 : public Flash
{
public:
    Flash128(const fs::path& file)
        : Flash(file, 0x20'000) {}
};
