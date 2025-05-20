#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include "chip8.h"

#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace std;

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

    return 0;
}

int main(int argc, char **argv) {
    Chip8 chip8;

    const double CYCLE_INTERVAL_MS = 1000.0 / 700.0;
    const double TIMER_INTERVAL_MS = 1000.0 / 60.0;

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

        if (cycle_accumulator >= CYCLE_INTERVAL_MS) {
            chip8.emulateCycle();
            cycle_accumulator -= CYCLE_INTERVAL_MS;
            if (chip8.drawFlag) {
                drawGraphics(chip8.getGfx(), renderer);
                chip8.drawFlag = false;
            }
        }

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                case SDL_EVENT_KEY_UP:
                    switch (event.key.key) {
                        case SDLK_ESCAPE:
                            running = false;
                            break;
                    }
            }
        }

        if (!running) {
            break;
        }
    }

    return 0;
}
