#include "Renderer.h"

int Renderer::Initialize(int _width, int _height) {
    //Setup
    width = _width;
    height = _height;

    //Initialize
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    //Create window
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    window = SDL_CreateWindow("chip8",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              (width * viewScaling),
                              (height * viewScaling),
                              SDL_WINDOW_SHOWN);
    if (window == NULL) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    //Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetScale(renderer, viewScaling, viewScaling);
    if (renderer == NULL) {
        std::cerr << "Renderer could not be created! SDL Error:" << SDL_GetError() << std::endl;
        return 1;
    } else {
        //Initialize renderer color
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    }
}

void Renderer::Terminate() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Renderer::Update(unsigned char _graphics[], bool _updateFrame) {
    if (_updateFrame) {
        draw(_graphics);
    }

    SDL_RenderPresent(renderer);
}

void Renderer::draw(unsigned char _graphics[]) {
    //Set background color
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    //Set draw point color
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (_graphics[(y * width) + x] == 1) {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}