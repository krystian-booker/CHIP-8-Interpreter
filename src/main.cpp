// System Headers
#include <iostream>
#include "chrono"
#include "thread"

// Local Headers
#include "Core.h"
#include "Renderer.h"
#include "Audio.h"

Core core;
Renderer renderer;
Audio audio;

int main(int argc, char *args[]) {
    //Initialize Audio
    SDL_Init(SDL_INIT_AUDIO);
    Audio::open();

    //Register input callbacks
    //setupInput();

    //Initialize the core, load rom into memory
    core.Initialize();
    core.LoadGame("Pong (1 player).ch8");

    renderer.Initialize(core.GetWidth(), core.GetHeight());

    bool gameActive = true;
    while (gameActive) {
        core.EmulateCycle();
        int status = renderer.Update(core.Graphics, core.DrawFlag);
        if (status != 0) {
            gameActive = false;
        }

        //TODO: Remove
        if(core.DrawFlag){
            core.DrawFlag = false;
        }

        if(core.BeepFlag){
            audio.Beep();
            core.BeepFlag = false;
        }

        std::this_thread::sleep_for (std::chrono::milliseconds(16)); // Sleep 16 ms
    }
    renderer.Terminate();

    return 0;
}