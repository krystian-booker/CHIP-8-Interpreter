// System Headers
#include <iostream>
#include "chrono"
#include "thread"

// Local Headers
#include "Core.h"
#include "Renderer.h"
Core core;
Renderer renderer;

int main(int argc, char *args[]) {
    //Register input callbacks
    //setupInput();

    //Initialize the core, load rom into memory
    core.Initialize();
    core.LoadGame("tetris.rom");

    renderer.Initialize(64, 32);

    bool gameActive = true;
    while (gameActive) {
        core.EmulateCycle();
        int status = renderer.Update(core.Graphics, core.DrawFlag);
        if (status != 0) {
            gameActive = false;
        }

        if(core.DrawFlag){
            /*for (int y = 0; y < 32; y++) {
                for (int x = 0; x < 64; x++) {
                    std::cout << (int) core.Graphics[y][x];
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
            std::cout << std::endl;
            std::cout << std::endl;*/

            core.DrawFlag = false;
        }
        std::this_thread::sleep_for (std::chrono::milliseconds(16)); // Sleep 16 ms
    }
    renderer.Terminate();

    return 0;
}