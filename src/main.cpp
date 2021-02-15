// System Headers
#include <iostream>

// Local Headers
#include "Core.h"

Core core;

int main() {
    //Register input callbacks
    //setupInput();

    // Initialize the Chip8 system and load the game into memory
    core.Initialize();
    core.LoadGame("test_opcode.ch8");

    // Emulation loop
    while (true) {
        // Emulate one cycle
        core.EmulateCycle();

        // If the draw flag is set, update the screen
        if (core.DrawFlag) {
            /*int columnCount = 0;
            int lineCount = 0;
            for (int i = 0; i < 2048; i++) {
                std::cout << (int) core.Graphics[i];
                columnCount++;
                if (columnCount == 64) {
                    lineCount++;
                    std::cout << ": end of line " << lineCount << std::endl;
                    columnCount = 0;
                }
            }*/

            core.DrawFlag = false;
        }

        // Store Key press state (Press and Release)
        //core.setKeys();
    }
    return 0;
}
