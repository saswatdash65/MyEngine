#pragma once
#include "sdlwindow.h"
#include "pipelinesetup.h"
#include <vulkan/vulkan.h>

extern int app_width, app_hieght;
extern VkPhysicalDeviceProperties properties;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    VkPresentModeKHR* presentModes;
} typedef SwapChainSupportDetails;

struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    int graphicsFamilyHasValue;
    int presentFamilyHasValue;
} typedef QueueFamilyIndices;
void initQueueFamilyIndices(QueueFamilyIndices* indices);
int isComplete(QueueFamilyIndices indices);

struct First_app {
    sdlWIndow window;

    const int enableValidationLayers;
    uint32_t validLayersCount;
    uint32_t deviceExtensionCount;

    VkInstance instance;
    #ifndef NDEBUG
    VkDebugUtilsMessengerEXT debugMessenger;
    #endif
    VkSurfaceKHR surface_;
    VkPhysicalDevice physicalDevice;
    VkDevice device_;
    VkCommandPool commandPool;
    CorePipeline pipeline;
    VkQueue graphicsQueue_;
    VkQueue presentQueue_;

    const char** deviceExtensions;
    const char** validationLayers;
} typedef  First_app;

void init_app(First_app* app, int width, int height);
void run_app(First_app *app);
void cleanupVulkan(First_app *app); //
void createInstance(First_app *app); //
#ifndef NDEBUG
void setupDebugMessenger(First_app* app);
void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator); //
#endif
void createSurface(First_app *app); //
void pickPhysicalDevice(First_app* app); //
void createLogicalDevice(First_app* app); //
void createCommandPool(First_app* app); //

VkFormat findSupportedFormat(const VkFormat* candidates, VkImageTiling tiling, VkFormatFeatureFlags features, First_app* app);
