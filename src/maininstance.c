#include "core_app.h"
#include "helpers.h"
#include <SDL2/SDL_timer.h>
#include <stdio.h>

int app_width = 800, app_hieght = 600;
VkPhysicalDeviceProperties properties;

int main(int argc, char* argv[]) {
    #ifndef NDEBUG
    const char* valLayer = "VK_LAYER_KHRONOS_validation";
    First_app app = {
        .enableValidationLayers = 1,
        .validLayersCount = 1,
        .deviceExtensionCount = 0,
        .deviceExtensions = NULL,
        .validationLayers = &valLayer
    };
    #else
    First_app app = {
        .enableValidationLayers = 0,
        .validLayersCount = 0,
        .deviceExtensionCount = 0,
        .deviceExtensions = NULL,
        .validationLayers = NULL
    };
    #endif
    init_app(&app, app_width, app_hieght);
    run_app(&app);
    return 0;
}
