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
    SDL_RenderSetScale(renderer, 8, 8);
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

int Renderer::Update(std::array<std::array<unsigned char, 64>, 32> _graphics, bool _updateFrame) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return 1;
        }
    }

    if(_updateFrame){
        draw(_graphics);
    }

    SDL_RenderPresent(renderer);
    return 0;
}

void Renderer::draw(std::array<std::array<unsigned char, 64>, 32> _graphics) {
    //Set background color
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    //Set draw point color
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    int rowNum;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            rowNum = y * 64;
            if(_graphics[y][x]){
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}