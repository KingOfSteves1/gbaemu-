#include "rtc.h"

#include <ctime>

#include <shell/utility.h>

#include "arm/arm.h"
#include "arm/constants.h"
#include "base/bit.h"
#include "base/log.h"

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
    if (isGbaToGpio(Port::kSio))
        SHELL_LOG_DEBUG("Bad SIO direction");

    return state == State::Transmit
        ? port.sio << 1
        : 0;
}

void Rtc::writePort(u16 half)
{
    if (isGbaToGpio(Port::kSck)) port.sck = bit::seq<Port::kSck, 1>(half); else SHELL_LOG_DEBUG("Bad SCK direction");
    if (isGbaToGpio(Port::kSio)) port.sio = bit::seq<Port::kSio, 1>(half);
    if (isGbaToGpio(Port::kCs))  port.cs  = bit::seq<Port::kCs,  1>(half); else SHELL_LOG_DEBUG("Bad CS direction");

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
        if (port.cs.low())
            setState(State::InitOne);
        else if (port.sck.rising())
            receiveCommand();
        break;

    case State::Receive:
        if (port.cs.low())
            setState(State::InitOne);
        else if (port.sck.rising())
            receiveData();
        break;

    case State::Transmit:
        if (port.cs.low())
            setState(State::InitOne);
        else if (port.sck.rising())
            transmitData();
        break;

    case State::Finalize:
        if (port.cs.falling())
            setState(State::InitOne);
        break;

    default:
        SHELL_UNREACHABLE;
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
    if (isGpioToGba(Port::kSio))
        SHELL_LOG_DEBUG("Bad SIO direction");

    buffer.pushl(port.sio);

    if (buffer.size < 8)
        return;

    constexpr uint kFixedBits = 0b0110;

    if (bit::seq<0, 4>(buffer.data) != kFixedBits &&
        bit::seq<4, 4>(buffer.data) == kFixedBits)
    {
        buffer = (buffer & 0xF0) >> 4 | (buffer & 0x0F) << 4;
        buffer = (buffer & 0xCC) >> 2 | (buffer & 0x33) << 2;
        buffer = (buffer & 0xAA) >> 1 | (buffer & 0x55) << 1;
        SHELL_LOG_DEBUG("Swapped command bits");
    }

    if (bit::seq<0, 4>(buffer.data) != kFixedBits)
    {
        SHELL_LOG_WARN("Bad command {:02X}", buffer.data);
        return;
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
    if (isGpioToGba(Port::kSio))
        SHELL_LOG_DEBUG("Bad SIO direction");

    buffer.pushl(port.sio);

    if (buffer.size < kDataBits[reg])
        return;

    writeRegister();

    setState(State::Finalize);
}

void Rtc::transmitData()
{
    if (isGbaToGpio(Port::kSio))
        SHELL_LOG_DEBUG("Bad SIO direction");

    port.sio = data.popr();

    if (data.size == 0)
        setState(State::Finalize);
}

void Rtc::readRegister()
{
    auto bcd = [](auto value) {
        return ((value / 10) << 4) | (value % 10);
    };

    auto stamp = std::time(NULL);
    auto time = *std::localtime(&stamp);

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
        data[0] = bcd(time.tm_year - 100);
        data[1] = bcd(time.tm_mon + 1);
        data[2] = bcd(time.tm_mday);
        data[3] = bcd(time.tm_wday);
        data[4] = bcd(time.tm_hour % (control.format_24h ? 24 : 12));
        data[5] = bcd(time.tm_min);
        data[6] = bcd(time.tm_sec);
        data.size = 56;
        break;

    case kRegTime:
        data[0] = bcd(time.tm_hour % (control.format_24h ? 24 : 12));
        data[1] = bcd(time.tm_min);
        data[2] = bcd(time.tm_sec);
        data.size = 24;
        break;

    default:
        SHELL_LOG_WARN("Bad register {}", reg);
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
        control = Control();
        break;

    case kRegForceIrq:
        arm.raise(kIrqGamePak);
        break;

    default:
        SHELL_LOG_WARN("Bad register {}", reg);
        break;
    }
}
