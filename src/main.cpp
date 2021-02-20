// System Headers
#include <iostream>
#include "chrono"
#include "thread"

// Local Headers
#include "Core.h"
#include "Renderer.h"
#include "Audio.h"
#include "Input.h"

Core core;
Renderer renderer;
Audio audio;
Input input;

int main(int argc, char *args[]) {
    if(!args[1]){
        std::cout << "Please enter a game" << std::endl;
    }

    //Initialize Audio
    SDL_Init(SDL_INIT_AUDIO);
    Audio::open();

    //Initialize the core, load rom into memory
    core.Initialize();
    core.LoadGame(args[1]);

    renderer.Initialize(core.GetWidth(), core.GetHeight());

    bool gameActive = true;
    while (gameActive) {
        core.EmulateCycle();

        //TODO: Refactor
        renderer.Update(core.Graphics, core.DrawFlag);
        if (core.DrawFlag) {
            core.DrawFlag = false;
        }

        int status = input.UpdateInput(core.Key);
        if (status != 0) {
            gameActive = false;
        }

        //TODO: Refactor
        if (core.BeepFlag) {
            audio.Beep();
            core.BeepFlag = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // Sleep 16 ms
    }
    renderer.Terminate();

    return 0;
}