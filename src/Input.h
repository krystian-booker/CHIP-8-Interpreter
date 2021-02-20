#ifndef CHIP8_INTERPRETER_INPUT_H
#define CHIP8_INTERPRETER_INPUT_H

#include <SDL2/SDL.h>
#include <iostream>

class Input {
public:
    static int UpdateInput(unsigned char Key[16]);
};


#endif //CHIP8_INTERPRETER_INPUT_H
