#include "gamepak.h"

#include "eeprom.h"
#include "flash.h"
#include "overwrites.h"
#include "rtc.h"
#include "sram.h"
#include "base/config.h"
#include "base/log.h"
#include "base/panic.h"

bool GamePak::isEepromAccess(u32 addr) const
{
    return save->type == Save::Type::Eeprom && rom.size < Rom::kSize
        ? addr >= 0xD00'0000 && addr < 0xE00'0000
        : addr >= 0xDFF'FF00 && addr < 0xE00'0000;
}

u8  GamePak::readByte(u32 addr) const { return read<u8 >(addr); }
u16 GamePak::readHalf(u32 addr) const { return read<u16>(addr); }
u32 GamePak::readWord(u32 addr) const { return read<u32>(addr); }

void GamePak::writeByte(u32 addr, u8  byte) { write(addr, byte); }
void GamePak::writeHalf(u32 addr, u16 half) { write(addr, half); }
void GamePak::writeWord(u32 addr, u32 word) { write(addr, word); }

void GamePak::load(fs::path gba, fs::path sav)
{
    if (gba.extension() == ".gba")
    {
        rom.load(gba);
        rom.mask = Rom::kSize;

        if (sav.empty())
        {
            sav = gba.replace_extension("sav");
            if (!config.save_path.empty())
                sav = config.save_path / sav.filename();
        }
    }

    Gpio::Type gpio_type = config.gpio;
    Save::Type save_type = config.save;

    if (const auto overwrite = Overwrite::find(rom.code))
    {
        gpio_type = overwrite->gpio;
        save_type = overwrite->save;
        rom.mask  = overwrite->mirror ? bit::ceilPow2(rom.size) : Rom::kSize;
    }

    gpio = gpio_type == Gpio::Type::None
        ? std::make_unique<Gpio>()
        : std::make_unique<Rtc>();

    if (sav.extension() == ".sav")
    {
        if (save_type == Save::Type::None)
            save_type =  Save::parse(rom);

        switch (save_type)
        {
        case Save::Type::Sram:      save = std::make_unique<Sram>(); break;
        case Save::Type::Eeprom:    save = std::make_unique<Eeprom>(); break;
        case Save::Type::Flash512:  save = std::make_unique<Flash>(Flash::kSize512); break;
        case Save::Type::Flash1024: save = std::make_unique<Flash>(Flash::kSize1024); break;
        default:                    save = std::make_unique<Save>(); break;
        }
        save->init(sav);
    }
}

template<typename Integral>
Integral GamePak::read(u32 addr) const
{
    addr &= rom.mask - sizeof(Integral);

    if (sizeof(Integral) > 1 
            && addr <= Gpio::kRegControl 
            && addr >= Gpio::kRegData
            && gpio->type != Gpio::Type::None
            && gpio->isReadable())
        return gpio->read(addr);

    return rom.read<Integral>(addr);
}

template<typename Integral>
void GamePak::write(u32 addr, Integral value)
{
    addr &= rom.mask - sizeof(Integral);

    if (sizeof(Integral) > 1
            && addr <= Gpio::kRegControl 
            && addr >= Gpio::kRegData
            && gpio->type != Gpio::Type::None)
        gpio->write(addr, value);
}
