// System Headers
#include "chrono"
#include "thread"

// Local Headers
#include "Core.h"
#include "Renderer.h"
#include "Audio.h"
#include "Input.h"

Core core;
Renderer renderer;

int main(int argc, char *args[]) {
    //Initialize Audio
    SDL_Init(SDL_INIT_AUDIO);
    Audio::open();

    //Initialize the core, load rom into memory
    core.Initialize();
    core.LoadGame("Blitz [David Winter].ch8");

    renderer.Initialize(core.GetWidth(), core.GetHeight());

    bool gameActive = true;
    while (gameActive) {
        core.EmulateCycle();

        //TODO: Refactor
        renderer.Update(core.Graphics, core.DrawFlag);
        if (core.DrawFlag) {
            core.DrawFlag = false;
        }

        int status = Input::UpdateInput(core.Key);
        if (status != 0) {
            gameActive = false;
        }

        if (core.BeepFlag) {
            Audio::Beep();
            core.BeepFlag = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // Sleep 16 ms
    }
    renderer.Terminate();

    return 0;
}