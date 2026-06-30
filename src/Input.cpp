#include "Input.h"

int Input::UpdateInput(unsigned char Key[16]) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                // KEYDOWN -> pressed (1), KEYUP -> released (0). Setting the
                // state explicitly (rather than toggling) avoids the key
                // flickering when SDL emits repeat KEYDOWN events while held.
                unsigned char pressed = (event.type == SDL_KEYDOWN) ? 1 : 0;
                switch (event.key.keysym.sym) {
                    case SDLK_x:
                        Key[0] = pressed;
                        break;
                    case SDLK_1:
                        Key[1] = pressed;
                        break;
                    case SDLK_2:
                        Key[2] = pressed;
                        break;
                    case SDLK_3:
                        Key[3] = pressed;
                        break;
                    case SDLK_q:
                        Key[4] = pressed;
                        break;
                    case SDLK_w:
                        Key[5] = pressed;
                        break;
                    case SDLK_e:
                        Key[6] = pressed;
                        break;
                    case SDLK_a:
                        Key[7] = pressed;
                        break;
                    case SDLK_s:
                        Key[8] = pressed;
                        break;
                    case SDLK_d:
                        Key[9] = pressed;
                        break;
                    case SDLK_z:
                        Key[10] = pressed;
                        break;
                    case SDLK_c:
                        Key[11] = pressed;
                        break;
                    case SDLK_4:
                        Key[12] = pressed;
                        break;
                    case SDLK_r:
                        Key[13] = pressed;
                        break;
                    case SDLK_f:
                        Key[14] = pressed;
                        break;
                    case SDLK_v:
                        Key[15] = pressed;
                        break;
                    default:
                        break;
                }
                break;
            }
            case SDL_QUIT:
                return 1;
            default:
                break;
        }
    }

    return 0;
}