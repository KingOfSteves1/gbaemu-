#include "core.h"

#include "mmu/memory_map.h"

Core::Core()
    : arm(mmu)
    , ppu(mmu)
{
    reset();
}

void Core::run(const std::string& file)
{
    if (!mmu.loadRom(file))
        return;

    if (!mmu.loadBios("bios.bin"))
        return;

    bool running = true;
    while (running)
    {
        frame();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_KEYDOWN:
                keyEvent(event.key.keysym.sym, true);
                break;

            case SDL_KEYUP:
                keyEvent(event.key.keysym.sym, false);
                break;
            }
        }
    }
}

void Core::reset()
{
    mmu.reset();
    arm.reset();
    ppu.reset();

    mmu.keyinput = 0xFF;
}

void Core::frame()
{
    // Visible lines
    for (int line = 0; line < 160; ++line)
    {
        ppu.scanline();
        runCycles(960);

        // H-Blank
        ppu.hblank();
        runCycles(272);

        ppu.next();
    }

    // Invisible lines and V-Blank
    ppu.vblank();
    for (int line = 0; line < 68; ++line)
    {
        runCycles(960 + 272);
        ppu.next();
    }

    ppu.render();
}

void Core::runCycles(int cycles)
{
    static int remaining = 0;
    remaining += cycles;

    while (remaining >= 0)
        remaining -= arm.step();
}

void Core::keyEvent(SDL_Keycode key, bool pressed)
{
    int state = pressed ? 0 : 1;

    switch (key)
    {
    case SDLK_u: mmu.keyinput.a      = state; break;
    case SDLK_h: mmu.keyinput.b      = state; break;
    case SDLK_f: mmu.keyinput.select = state; break;
    case SDLK_g: mmu.keyinput.start  = state; break;
    case SDLK_d: mmu.keyinput.right  = state; break;
    case SDLK_a: mmu.keyinput.left   = state; break;
    case SDLK_w: mmu.keyinput.up     = state; break;
    case SDLK_s: mmu.keyinput.down   = state; break;
    case SDLK_i: mmu.keyinput.r      = state; break;
    case SDLK_q: mmu.keyinput.l      = state; break;
    }
}
