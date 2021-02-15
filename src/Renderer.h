#ifndef CHIP8_INTERPRETER_RENDERER_H
#define CHIP8_INTERPRETER_RENDERER_H
#include <iostream>
#include <SDL2/SDL.h>
#include <array>

class Renderer {
public:
    int Initialize(int _width, int _height);
    int Update(std::array<std::array<unsigned char, 64>, 32> _graphics, bool _updateFrame);

    void Terminate();
private:
    int width = 0;
    int height = 0;
    int viewScaling = 8;

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    void draw(std::array<std::array<unsigned char, 64>, 32> _graphics);
};


#endif //CHIP8_INTERPRETER_RENDERER_H
