#include "base/eggcpt.h"

#if !EGGCPT_CC_EMSCRIPTEN

#include <stdexcept>

#include "base/config.h"
#include "core/core.h"
#include "core/framecounter.h"
#include "core/synchronizer.h"

Core core;
bool running = true;
FrameCounter counter;
Synchronizer synchronizer(kRefreshRate);

void init(int argc, char* argv[])
{
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

    core.init(argc, argv);
}

void processDropEvent(const SDL_DropEvent& event)
{
    auto file = fs::u8path(event.file);

    SDL_free(event.file);

    core.context.video.raise();

    if (file.extension() == ".gba")
    {
        core.mmu.gamepak.load(file);
        core.reset();
        core.updateTitle();
        counter = FrameCounter();
    }
    else
    {
        core.mmu.gamepak.loadSave(file);
        core.reset();
    }
}

template<typename T>
void processInputEvent(const Shortcuts<T>& shortcuts, T input)
{
    if (input == shortcuts.reset)
        core.reset();

    if (input == shortcuts.fullscreen)
        core.context.video.fullscreen();

    if (input == shortcuts.fr_hardware)
        synchronizer = Synchronizer(kRefreshRate);

    if (input == shortcuts.fr_custom_1)
        synchronizer = Synchronizer(config.framerate[0]);

    if (input == shortcuts.fr_custom_2)
        synchronizer = Synchronizer(config.framerate[1]);

    if (input == shortcuts.fr_custom_3)
        synchronizer = Synchronizer(config.framerate[2]);

    if (input == shortcuts.fr_custom_4)
        synchronizer = Synchronizer(config.framerate[3]);

    if (input == shortcuts.fr_unbound)
        synchronizer = Synchronizer(6000);
}

void processEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            running = false;
            break;

        case SDL_KEYDOWN:
            processInputEvent(config.shortcuts.keyboard, event.key.keysym.scancode);
            break;

        case SDL_CONTROLLERBUTTONDOWN:
            processInputEvent(config.shortcuts.controller, SDL_GameControllerButton(event.cbutton.button));
            break;

        case SDL_CONTROLLERDEVICEADDED:
        case SDL_CONTROLLERDEVICEREMOVED:
            core.context.input.processDeviceEvent(event.cdevice);
            break;

        case SDL_DROPFILE:
            processDropEvent(event.drop);
            break;
        }
    }
}

void emulate()
{
    while (running && core.mmu.gamepak.size() == 0)
    {
        processEvents();

        core.context.video.renderClear(0x2B3137);
        core.context.video.renderIcon();
        core.context.video.renderPresent();

        SDL_Delay(16);
    }

    core.reset();
    core.updateTitle();

    counter = FrameCounter();

    while (running)
    {
        synchronizer.sync([]()
        {
            processEvents();
            core.keypad.update();
            core.frame();
        });

        if (auto fps = (++counter).fps())
            core.updateTitle(*fps);
    }
}

int main(int argc, char* argv[])
{
    try
    {
        init(argc, argv);
        emulate();
        return 0;
    }
    catch (const std::exception& ex)
    {
        SDL_ShowSimpleMessageBox(0, "Exception", ex.what(), nullptr);
        return 1;
    }
}

#endif