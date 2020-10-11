#include "gamepak.h"

#include "base/config.h"
#include "base/utility.h"
#include "gamepak/eeprom.h"
#include "gamepak/flash.h"
#include "gamepak/rtc.h"
#include "gamepak/sram.h"

std::size_t GamePak::size() const
{
    return rom.size();
}

u8 GamePak::readByte(u32 addr) const
{
    return read<u8>(addr);
}

u16 GamePak::readHalf(u32 addr) const
{
    return read<u16>(addr);
}

u32 GamePak::readWord(u32 addr) const
{
    return read<u32>(addr);
}

void GamePak::loadRom(const fs::path& file, bool load_save)
{
    if (!fs::read(file, rom))
    {
        message("Cannot read rom: {}", file);
        return;
    }

    header = Header(rom);

    const auto overwrite = Overwrite::find(header.code);

    initGpio(overwrite ? overwrite->gpio : config.gpio);

    if (load_save)
    {
        fs::path save(file);
        save.replace_extension("sav");

        if (!config.save_path.empty())
            save = config.save_path / save.filename();

        loadSave(save);
    }
}

void GamePak::loadSave(const fs::path& file)
{
    if (rom.empty())
        return;

    Save::Type type = config.save;

    if (const auto overwrite = Overwrite::find(header.code))
        type = overwrite->save;

    initSave(file, type == Save::Type::None ? Save::parse(rom) : type);
}

u32 GamePak::readUnused(u32 addr)
{
    addr = (addr & ~0x3) >> 1;
    return (addr & 0xFFFF) | ((addr + 1) & 0xFFFF) << 16;
}

template<typename T>
T GamePak::read(u32 addr) const
{
    addr &= 0x200'0000 - sizeof(T);

    if (addr < rom.size())
        return *reinterpret_cast<const T*>(&rom[addr]);
    else
        return readUnused(addr);
}

void GamePak::initGpio(Gpio::Type type)
{
    gpio = std::invoke([&]() -> std::unique_ptr<Gpio>
    {
        if (type == Gpio::Type::Rtc)
            return std::make_unique<Rtc>();
        
        return std::make_unique<Gpio>();
    });
}

void GamePak::initSave(const fs::path& file, Save::Type type)
{
    save = std::invoke([&]() -> std::unique_ptr<Save>
    {
        switch (type)
        {
        case Save::Type::Sram:      return std::make_unique<Sram>();
        case Save::Type::Eeprom:    return std::make_unique<Eeprom>();
        case Save::Type::Flash512:  return std::make_unique<Flash>(Flash::kSize512);
        case Save::Type::Flash1024: return std::make_unique<Flash>(Flash::kSize1024);
        }
        return std::make_unique<Save>();
    });

    if (!save->init(file))
    {
        const auto overwrite = Overwrite::find(header.code);

        initSave(fs::path(),overwrite ? overwrite->save : Save::parse(rom));
    }
}
