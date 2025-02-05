#include "core_app.h"
#include "helpers.h"
#include "sdlwindow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL_vulkan.h>

void init_app(First_app* app, int width, int height) {
    initWindow(&app->window, width, height);
    createInstance(app);
    #ifndef NDEBUG
    setupDebugMessenger(app);
    #endif
    createSurface(app);
    pickPhysicalDevice(app);
    createLogicalDevice(app);
    createCommandPool(app);
    createGraphicsPipeline(&app->pipeline, "./shaders/vs_shader.spv", "./shaders/ps_shader.spv");

}

void run_app(First_app *app) {
    mainLoop(&app->window);
    cleanupVulkan(app);
    cleanup(&app->window);
}

void initQueueFamilyIndices(QueueFamilyIndices* indices) {
    indices->graphicsFamily = 0, indices->presentFamily = 0;
    indices->graphicsFamilyHasValue = 0, indices->presentFamilyHasValue = 0;
}
int isComplete(QueueFamilyIndices indices) { return indices.graphicsFamilyHasValue && indices.presentFamilyHasValue; }

void cleanupVulkan(First_app *app) {
    vkDestroyCommandPool(app->device_, app->commandPool, NULL);
    vkDestroyDevice(app->device_, NULL);

    #ifndef NDEBUG
    if (app->enableValidationLayers) {
      DestroyDebugUtilsMessengerEXT(app->instance, app->debugMessenger, NULL);
    }
    #endif

    vkDestroySurfaceKHR(app->instance, app->surface_, NULL);
    vkDestroyInstance(app->instance, NULL);
}

#ifndef NDEBUG
void setupDebugMessenger(First_app* app) {
    if (!app->enableValidationLayers) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(&createInfo);
    if (CreateDebugUtilsMessengerEXT(app->instance, &createInfo, NULL, &app->debugMessenger) != VK_SUCCESS) {
        perror("Failed to create debug messanger\n");
        vkDestroyInstance(app->instance, NULL);
        cleanup(&app->window);
        exit(1);
    }
    printf("Created debug messanger\n");
}
#endif

void createInstance(First_app *app) {
    #ifndef NDEBUG
    if (app->enableValidationLayers && !checkValidationLayerSupport(app)) {
        cleanup(&app->window);
        exit(1);
    }
    #endif

    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "LittleVulkanEngine App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t extensionCount;
    const char** extensions = getRequiredExtensions(&extensionCount, &app->window);
    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;

    #ifndef NDEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (app->enableValidationLayers) {
        createInfo.enabledLayerCount = app->validLayersCount;
        createInfo.ppEnabledLayerNames = app->validationLayers;

        populateDebugMessengerCreateInfo(&debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
        printf("Assigned debug info\n");
    }
    #else
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = NULL;
    #endif
    if (vkCreateInstance(&createInfo, NULL, &app->instance) != VK_SUCCESS) {
        free(extensions);
        cleanup(&app->window);
        exit(1);
    }
    free(extensions);
    #ifndef NDEBUG
    printf("Created instance\n");
    #endif
}

void createSurface(First_app *app) {
    if(SDL_Vulkan_CreateSurface(app->window.window, app->instance, &app->surface_) != SDL_TRUE) {
        perror("Failed to create surface\n");
        vkDestroyInstance(app->instance, NULL);
        cleanup(&app->window);
        exit(1);
    }
}

void pickPhysicalDevice(First_app* app) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(app->instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        exit(1);
    }
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(app->instance, &deviceCount, devices);
    for (uint32_t i = 0; i < deviceCount; i++) {
        if (isDeviceSuitable(devices[i], app)) {
            app->physicalDevice = devices[i];
            break;
        }
    }

    if (app->physicalDevice == VK_NULL_HANDLE) {
        perror("No suitable device found\n");
        vkDestroySurfaceKHR(app->instance, app->surface_, NULL);
        vkDestroyInstance(app->instance, NULL);
        cleanup(&app->window);
        exit(1);
    }

    #ifndef NDEBUG
    vkGetPhysicalDeviceProperties(app->physicalDevice, &properties);
    printf("physical device: %s\n", properties.deviceName);
    #endif
}

void createLogicalDevice(First_app* app) {
    QueueFamilyIndices indices = findQueueFamilies(app->physicalDevice, app);

    uint32_t size = indices.graphicsFamily == indices.presentFamily ? 1 : 2;
    VkDeviceQueueCreateInfo queueCreateInfos[size];
    uint32_t uniqueQueueFamilies[] = {indices.graphicsFamily, indices.presentFamily};

    float queuePriority = 1.0f;
    for (uint32_t i = 0; i < size; i++) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[i] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = size;
    createInfo.pQueueCreateInfos = queueCreateInfos;

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = app->deviceExtensionCount;
    createInfo.ppEnabledExtensionNames = app->deviceExtensions;

    // might not really be necessary anymore because device specific validation layers
    // have been deprecated
    if (app->enableValidationLayers) {
        createInfo.enabledLayerCount = app->validLayersCount;
        createInfo.ppEnabledLayerNames = app->validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(app->physicalDevice, &createInfo, NULL, &app->device_) != VK_SUCCESS) {
        perror("Couldn't create logical device\n");
        vkDestroySurfaceKHR(app->instance, app->surface_, NULL);
        vkDestroyInstance(app->instance, NULL);
        cleanup(&app->window);
        exit(1);
    }

    vkGetDeviceQueue(app->device_, indices.graphicsFamily, 0, &app->graphicsQueue_);
    vkGetDeviceQueue(app->device_, indices.presentFamily, 0, &app->presentQueue_);
}

void createCommandPool(First_app* app) {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(app->physicalDevice, app);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(app->device_, &poolInfo, NULL, &app->commandPool) != VK_SUCCESS) {
        perror("Couldn't create logical device\n");
        vkDestroyDevice(app->device_, NULL);
        vkDestroySurfaceKHR(app->instance, app->surface_, NULL);
        vkDestroyInstance(app->instance, NULL);
        cleanup(&app->window);
        exit(1);
    }
}

VkFormat findSupportedFormat(const VkFormat* candidates, VkImageTiling tiling, VkFormatFeatureFlags features, First_app* app) {
   return VK_FORMAT_B8G8R8A8_SINT;
}
