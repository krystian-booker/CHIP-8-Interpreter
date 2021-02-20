#include "Input.h"

int Input::UpdateInput(unsigned char Key[16]) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                    case SDLK_1:
                        Key[0] = !Key[0];
                        break;
                    case SDLK_2:
                        Key[1] = !Key[1];
                        break;
                    case SDLK_3:
                        Key[2] = !Key[2];
                        break;
                    case SDLK_4:
                        Key[3] = !Key[3];
                        break;
                    case SDLK_q:
                        Key[4] = !Key[4];
                        break;
                    case SDLK_w:
                        Key[5] = !Key[5];
                        break;
                    case SDLK_e:
                        Key[6] = !Key[6];
                        break;
                    case SDLK_r:
                        Key[7] = !Key[7];
                        break;
                    case SDLK_a:
                        Key[8] = !Key[8];
                        break;
                    case SDLK_s:
                        Key[9] = !Key[9];
                        break;
                    case SDLK_d:
                        Key[10] = !Key[10];
                        break;
                    case SDLK_f:
                        Key[11] = !Key[11];
                        break;
                    case SDLK_z:
                        Key[12] = !Key[12];
                        break;
                    case SDLK_x:
                        Key[13] = !Key[13];
                        break;
                    case SDLK_c:
                        Key[14] = !Key[14];
                        break;
                    case SDLK_v:
                        Key[15] = !Key[15];
                        break;
                    default:
                        break;
                }
                break;
            case SDL_QUIT:
                return 1;
            default:
                break;
        }
    }
}