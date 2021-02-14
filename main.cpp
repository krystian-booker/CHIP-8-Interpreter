#include <iostream>
#include "Core.h"

Core core;

int main() {
    // Set up render system and register input callbacks
    //setupGraphics();
    //setupInput();

    // Initialize the Chip8 system and load the game into memory
    core.initialize();
    core.loadGame("chip8-test-rom.ch8");

    // Emulation loop
    for (;;) {
        // Emulate one cycle
        core.emulateCycle();

        // If the draw flag is set, update the screen
        //if(core.drawFlag){
        //drawGraphics();
        //}

        // Store key press state (Press and Release)
        //core.setKeys();
    }

    return 0;
}
