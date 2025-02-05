#pragma once
#include <SDL2/SDL_video.h>

struct sdlWindow {
    SDL_Window *window;
    int width, height;

    char* window_name;
} typedef sdlWIndow;

void initWindow(sdlWIndow* app, int width, int height);
void cleanup(sdlWIndow* app);
void mainLoop(sdlWIndow* app);
