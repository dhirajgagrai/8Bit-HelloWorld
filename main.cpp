#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include "chip8.h"

#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace std;

Chip8 chip8;

// TODO: rewrite this function because it was generated with chatgpt
int setupGraphics(SDL_Window *&window, SDL_Renderer *&renderer) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return -1;
    }

    window = SDL_CreateWindow("CHIP-8 Emulator", 640, 320, SDL_WINDOW_RESIZABLE);
    if (!window) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
        return -1;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << "\n";
        return -1;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    return 0;
}

// TODO: rewrite this function because it was generated with chatgpt
int drawGraphics(const unsigned char *gfx, SDL_Renderer *&renderer) {
    const int VIDEO_WIDTH  = 64;
    const int VIDEO_HEIGHT = 32;
    const int PIXEL_SCALE  = 10;

    SDL_SetRenderDrawColor(renderer, 155, 103, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 204, 2, 255);

    for (int y = 0; y < VIDEO_HEIGHT; ++y) {
        for (int x = 0; x < VIDEO_WIDTH; ++x) {
            if (gfx[y * VIDEO_WIDTH + x]) {
                SDL_FRect rect = {static_cast<float>(x * PIXEL_SCALE),
                                  static_cast<float>(y * PIXEL_SCALE),
                                  static_cast<float>(PIXEL_SCALE), static_cast<float>(PIXEL_SCALE)};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    SDL_RenderPresent(renderer);

    chip8.drawFlag = false;

    return 0;
}

int handleInput(SDL_Event event, bool *running) {
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                *running = false;
                break;
            case SDL_EVENT_KEY_UP:
                switch (event.key.key) {
                    case SDLK_ESCAPE:
                        *running = false;
                        break;
                    case SDLK_1:
                        chip8.unsetKey(1);
                        break;
                    case SDLK_2:
                        chip8.unsetKey(2);
                        break;
                    case SDLK_3:
                        chip8.unsetKey(3);
                        break;
                    case SDLK_4:
                        chip8.unsetKey(0xC);
                        break;
                    case SDLK_Q:
                        chip8.unsetKey(4);
                        break;
                    case SDLK_W:
                        chip8.unsetKey(5);
                        break;
                    case SDLK_E:
                        chip8.unsetKey(6);
                        break;
                    case SDLK_R:
                        chip8.unsetKey(0xD);
                        break;
                    case SDLK_A:
                        chip8.unsetKey(7);
                        break;
                    case SDLK_S:
                        chip8.unsetKey(8);
                        break;
                    case SDLK_D:
                        chip8.unsetKey(9);
                        break;
                    case SDLK_F:
                        chip8.unsetKey(0xE);
                        break;
                    case SDLK_Z:
                        chip8.unsetKey(0xA);
                        break;
                    case SDLK_X:
                        chip8.unsetKey(0);
                        break;
                    case SDLK_C:
                        chip8.unsetKey(0xB);
                        break;
                    case SDLK_V:
                        chip8.unsetKey(0xF);
                        break;
                    default:
                        break;
                }
                break;
            case SDL_EVENT_KEY_DOWN: {
                switch (event.key.key) {
                    case SDLK_1:
                        chip8.setKey(1);
                        break;
                    case SDLK_2:
                        chip8.setKey(2);
                        break;
                    case SDLK_3:
                        chip8.setKey(3);
                        break;
                    case SDLK_4:
                        chip8.setKey(0xC);
                        break;
                    case SDLK_Q:
                        chip8.setKey(4);
                        break;
                    case SDLK_W:
                        chip8.setKey(5);
                        break;
                    case SDLK_E:
                        chip8.setKey(6);
                        break;
                    case SDLK_R:
                        chip8.setKey(0xD);
                        break;
                    case SDLK_A:
                        chip8.setKey(7);
                        break;
                    case SDLK_S:
                        chip8.setKey(8);
                        break;
                    case SDLK_D:
                        chip8.setKey(9);
                        break;
                    case SDLK_F:
                        chip8.setKey(0xE);
                        break;
                    case SDLK_Z:
                        chip8.setKey(0xA);
                        break;
                    case SDLK_X:
                        chip8.setKey(0);
                        break;
                    case SDLK_C:
                        chip8.setKey(0xB);
                        break;
                    case SDLK_V:
                        chip8.setKey(0xF);
                        break;
                    default:
                        break;
                }
                break;
            }
        }
    }

    return 0;
}

int main(int argc, char **argv) {

    const double TIMER_INTERVAL_MS = 1000.0 / 60.0;
    const int    CYCLES_PER_FRAME  = 10;

    bool running = true;

    SDL_Renderer *renderer = nullptr;
    SDL_Window   *window   = nullptr;
    SDL_Event     event;

    if (setupGraphics(window, renderer) != 0) {
        exit(1);
    }

    cout << "*** Chip-8 log ***\n";

    chip8.initialize();
    chip8.loadGame(argv[1]);

    using clock              = chrono::high_resolution_clock;
    auto   last_time         = clock::now();
    auto   current_time      = clock::now();
    double elapsed           = 0.0;
    double cycle_accumulator = 0.0;
    double timer_accumulator = 0.0;

    for (;;) {
        current_time       = clock::now();
        elapsed            = chrono::duration<double, milli>(current_time - last_time).count();
        elapsed            = min(elapsed, 100.0);
        last_time          = current_time;
        cycle_accumulator += elapsed;
        timer_accumulator += elapsed;

        if (timer_accumulator >= TIMER_INTERVAL_MS) {
            chip8.tickTimers();
            timer_accumulator -= TIMER_INTERVAL_MS;
        }

        for (int i = 0; i < CYCLES_PER_FRAME; i++) {
            chip8.emulateCycle();
        }

        if (chip8.drawFlag) {
            drawGraphics(chip8.getGfx(), renderer);
        }

        handleInput(event, &running);

        if (!running) {
            break;
        }
    }

    return 0;
}
