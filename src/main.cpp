// System Headers
#include <iostream>

// Local Headers
#include "Core.h"
#include "Renderer.h"

Core core;
Renderer renderer;

int main() {
    // Set up render system and
    int status = renderer.Initialize(64, 32);
    if(status != 0){
        std::cerr << "Issues initialize window" << std::endl;
    }

    //Register input callbacks
    //setupInput();

    // Initialize the Chip8 system and load the game into memory
    core.initialize();
    core.loadGame("chip8-test-rom.ch8");

    // Emulation loop
    while (!renderer.WindowShouldClose()) {
        // Emulate one cycle
        core.emulateCycle();

        // If the draw flag is set, update the screen
        if (core.drawFlag) {
            /*int columnCount = 0;
            int lineCount = 0;
            for (int i = 0; i < 2048; i++) {
                std::cout << (int) core.gfx[i];
                columnCount++;
                if (columnCount == 64) {
                    lineCount++;
                    std::cout << ": end of line " << lineCount << std::endl;
                    columnCount = 0;
                }
            }*/

            //renderer.drawGraphics();
            core.drawFlag = false;
        }

        // Store key press state (Press and Release)
        //core.setKeys();

        renderer.BackgroundColor();

        renderer.Draw();

        renderer.Update();
    }
    renderer.Terminate();
    return 0;
}
