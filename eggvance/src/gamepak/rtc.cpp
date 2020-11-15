#include "rtc.h"

#include "arm/arm.h"
#include "arm/constants.h"
#include "base/bit.h"

#include <shell/utility.h>

Rtc::Rtc()
    : Gpio(Type::Rtc)
{

}

void Rtc::reset()
{
    shell::reconstruct(*this);
}

u16 Rtc::readPort()
{
    return state == State::Transmit
        ? port.sio << kBitSio
        : 1;
}

void Rtc::writePort(u16 half)
{
    if (isGbaToGpio(kBitCs))  port.cs  = bit::seq<kBitCs,  1>(half);
    if (isGbaToGpio(kBitSio)) port.sio = bit::seq<kBitSio, 1>(half);
    if (isGbaToGpio(kBitSck)) port.sck = bit::seq<kBitSck, 1>(half);

    switch (state)
    {
    case State::InitOne:
        if (port.cs.low() && port.sck.high())
            setState(State::InitTwo);
        break;

    case State::InitTwo:
        if (port.cs.rising())
            setState(State::Command);
        break;

    case State::Command:
        if (port.cs.high() && port.sck.rising())
            receiveCommand();
        break;

    case State::Receive:
        if (port.cs.high() && port.sck.rising())
            receiveData();
        break;

    case State::Transmit:
        if (port.cs.high() && port.sck.rising())
            transmitData();
        break;

    case State::Finalize:
        if (port.cs.falling())
            setState(State::InitOne);
        break;
    }
}

void Rtc::setState(State state)
{
    this->state = state;
    this->buffer.clear();
}

void Rtc::receiveCommand()
{
    buffer.pushl(port.sio);

    if (buffer.size < 8)
        return;

    if ((buffer >> 4) == 6)
    {
        buffer = (buffer & 0xF0) >> 4 | (buffer & 0x0F) << 4;
        buffer = (buffer & 0xCC) >> 2 | (buffer & 0x33) << 2;
        buffer = (buffer & 0xAA) >> 1 | (buffer & 0x55) << 1;
    }

    reg = bit::seq<4, 3>(buffer.data);

    if (buffer & 0x80)
    {
        readRegister();

        setState(kDataBits[reg] > 0
            ? State::Transmit
            : State::Finalize);
    }
    else
    {
        if (kDataBits[reg] > 0)
        {
            setState(State::Receive);
        }
        else
        {
            writeRegister();

            setState(State::Finalize);
        }
    }
}

void Rtc::receiveData()
{
    buffer.pushl(port.sio);

    if (buffer.size == kDataBits[reg])
    {
        writeRegister();

        setState(State::Finalize);
    }
}

void Rtc::transmitData()
{
    port.sio = data.popr();

    if (data.size == 0)
        setState(State::Finalize);
}

std::tm Rtc::readBcdTime() const
{
    auto toBcd = [](uint decimal) {
        return ((decimal / 10) << 4) | (decimal % 10);
    };

    auto time = std::time(NULL);
    auto bcd = *std::localtime(&time);

    bcd.tm_year = toBcd(bcd.tm_year - 100);
    bcd.tm_mon  = toBcd(bcd.tm_mon + 1);
    bcd.tm_mday = toBcd(bcd.tm_mday);
    bcd.tm_wday = toBcd(bcd.tm_wday);
    bcd.tm_hour = toBcd(bcd.tm_hour % (control.format_24h ? 24 : 12));
    bcd.tm_min  = toBcd(bcd.tm_min);
    bcd.tm_sec  = toBcd(bcd.tm_sec);

    return bcd;
}

void Rtc::readRegister()
{
    auto time = readBcdTime();

    switch (reg)
    {
    case kRegControl:
        data[0] = 0;
        data[0] |= control.unknown1   << 1;
        data[0] |= control.minute_irq << 3;
        data[0] |= control.unknown2   << 5;
        data[0] |= control.format_24h << 6;
        data.size = 8;
        break;

    case kRegDateTime:
        data[0] = time.tm_year;
        data[1] = time.tm_mon;
        data[2] = time.tm_mday;
        data[3] = time.tm_wday;
        data[4] = time.tm_hour;
        data[5] = time.tm_min;
        data[6] = time.tm_sec;
        data.size = 56;
        break;

    case kRegTime:
        data[0] = time.tm_hour;
        data[1] = time.tm_min;
        data[2] = time.tm_sec;
        data.size = 24;
        break;
    }
}

void Rtc::writeRegister()
{
    switch (reg)
    {
    case kRegControl:
        control.unknown1   = bit::seq<1, 1>(buffer.data);
        control.minute_irq = bit::seq<3, 1>(buffer.data);
        control.unknown2   = bit::seq<5, 1>(buffer.data);
        control.format_24h = bit::seq<6, 1>(buffer.data);
        break;

    case kRegForceReset:
        control = {};
        break;

    case kRegForceIrq:
        arm.raise(kIrqGamePak);
        break;
    }
}
