#include "timer.h"

#include "arm/arm.h"

void Timer::reset()
{
    control.reset();

    counter  = 0;
    overflow = 0;
}

void Timer::run(int cycles)
{
    if (!control.enabled || control.cascade)
        return;

    runInternal(cycles);
}

u8 Timer::readByte(int index)
{
    if (index <= 1)
        updateData();

    return control.readByte(index);
}

void Timer::writeByte(int index, u8 byte)
{
    updateData();

    int enabled = control.enabled;
    control.writeByte(index, byte);
    if (!enabled && control.enabled)
        control.data = control.initial;

    calculate();
}

void Timer::runInternal(int cycles)
{
    counter += cycles;

    // Todo: check if multiple overflows?
    if (counter >= overflow)
    {
        if (next && next->control.cascade)
            next->runInternal(1);

        if (control.irq)
        {
            static constexpr Interrupt flags[4] = {
                Interrupt::Timer0,
                Interrupt::Timer1,
                Interrupt::Timer2,
                Interrupt::Timer3
            };
            arm.request(flags[id]);
        }

        counter = prescalers[control.prescaler] * control.initial + (counter % overflow);
    }
}

void Timer::updateData()
{
    control.data = counter / prescalers[control.prescaler];
}

void Timer::calculate()
{
    overflow = prescalers[control.prescaler] * 0x1'0000;
    counter  = prescalers[control.prescaler] * control.data;
}
