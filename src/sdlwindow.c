#include "sdlwindow.h"
#include <SDL2/SDL.h>
#include <stdint.h>

void initWindow(sdlWIndow* app, int width, int height) {
    SDL_Init(SDL_INIT_VIDEO);

    app->window = SDL_CreateWindow(
         "Vulkan window",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_VULKAN);
}

void cleanup(sdlWIndow *app) {
    SDL_DestroyWindow(app->window);
    SDL_Quit();
}

void mainLoop(sdlWIndow *app) {
    SDL_Event event;
    uint8_t running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
            }
        }
    }
}
