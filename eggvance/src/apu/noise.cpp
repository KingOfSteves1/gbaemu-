#include "noise.h"

#include "constants.h"

void Noise::init()
{
    noise = 0x4000 >> shift;

    length.init();
    envelope.init();

    enabled = envelope.enabled();

    updateTimer();
}

void Noise::tick()
{
    if (!(enabled && timer && --timer == 0))
        return;

    sample = noise & 0x1;
    sample *= envelope.volume;

    noise >>= 1;
    if (sample)
        noise ^= 0x6000 >> shift;

    updateTimer();
}

void Noise::tickLength()
{
    if (enabled)
    {
        length.tick();

        enabled = length.enabled();
    }
}

void Noise::tickEnvelope()
{
    if (enabled)
    {
        envelope.tick();

        enabled = envelope.enabled();
    }
}

void Noise::updateTimer()
{
    timer = kCpuFrequency / frequency;
}
