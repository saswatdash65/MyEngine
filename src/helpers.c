#include "helpers.h"
#include "core_app.h"
#include "sdlwindow.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>
#ifdef __unix
#include <sys/stat.h>
#endif

char* readShader(const char* filePath) {
    FILE* opened_file = fopen(filePath, "rb");
    size_t fileSize = 0;
    if (!opened_file) {
        fprintf(stderr, "File %s doesn't exist\n", filePath);
        return NULL;
    }

    #ifdef __unix
    struct stat st;
    stat(filePath, &st);
    fileSize = st.st_size;
    #else
    fseek(opened_file, 0, SEEK_END);
    fileSize = ftell(opened_file);
    rewind(opened_file);
    #endif

    char* buffer = (char*)malloc(fileSize);
    fread(buffer, 1, fileSize, opened_file);
    fclose(opened_file);
    return buffer;
}

#ifndef NDEBUG
// Local Debug helpers
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {

    printf("Validationn layer: %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance,
        "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator) {

    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance,
        "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, debugMessenger, pAllocator);
    }
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *createInfo) {
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
    createInfo->pUserData = NULL;  // Optional
}
#endif

int checkValidationLayerSupport(First_app* app) {
    uint32_t layerCount;
    int layerFound = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
    for (int i = 0; i < app->validLayersCount; ++i) {
        for (uint32_t j = 0; j < layerCount; j++) {
            if (strcmp(app->validationLayers[i], availableLayers[j].layerName) == 0) {
                layerFound = 1;
                break;
            }
        }
    }
    return layerFound;
}

int isDeviceSuitable(VkPhysicalDevice device, First_app* app) {
    QueueFamilyIndices indices = findQueueFamilies(device, app);

    int extensionsSupported = checkDeviceExtensionSupport(device, app);
    int swapChainAdequate = 0;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, app);
        swapChainAdequate = swapChainSupport.formats && swapChainSupport.presentModes;
        free(swapChainSupport.formats);
        free(swapChainSupport.presentModes);
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.graphicsFamilyHasValue && indices.presentFamilyHasValue && extensionsSupported && swapChainAdequate &&
           supportedFeatures.samplerAnisotropy;
}

const char** getRequiredExtensions(uint32_t* extensionCount, sdlWIndow* app) {
    SDL_Vulkan_GetInstanceExtensions(app->window, extensionCount, NULL);
    const char** extensions = (const char**)malloc(*extensionCount * sizeof(const char*));
    SDL_Vulkan_GetInstanceExtensions(app->window, extensionCount, extensions);
    #ifndef NDEBUG
    *extensionCount += 1;
    extensions = (const char**)realloc(extensions, *extensionCount * sizeof(const char*));
    extensions[*extensionCount-1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    #endif
    return extensions;
}

int checkDeviceExtensionSupport(VkPhysicalDevice device,First_app* app) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

    VkExtensionProperties availableExtensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    uint32_t tempCount = 0;
    for (int i = 0; i < app->deviceExtensionCount; i++) {
        for (int j = 0; j < extensionCount; j++)
            tempCount += strcmp(availableExtensions[j].extensionName, app->deviceExtensions[i]) ? 1 : 0;
    }

    return tempCount == app->deviceExtensionCount;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, First_app* app) {
  QueueFamilyIndices indices;
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

  VkQueueFamilyProperties queueFamilies[queueFamilyCount];
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

  int i = 0;
  for (; i < queueFamilyCount;) {
    if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
      indices.graphicsFamilyHasValue = 1;
    }
    VkBool32 presentSupport = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, app->surface_, &presentSupport);
    if (queueFamilies[i].queueCount > 0 && presentSupport) {
      indices.presentFamily = i;
      indices.presentFamilyHasValue = 1;
    }
    if (indices.presentFamilyHasValue && indices.graphicsFamilyHasValue) {
      break;
    }
    i++;
  }
  return indices;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, First_app* app) {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, app->surface_, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, app->surface_, &formatCount, NULL);

    if (formatCount != 0) {
        details.formats = (VkSurfaceFormatKHR*)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, app->surface_, &formatCount, details.formats);
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, app->surface_, &presentModeCount, NULL);

    if (presentModeCount != 0) {
        details.presentModes = (VkPresentModeKHR*)malloc(presentModeCount * sizeof(VkPresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            app->surface_,
            &presentModeCount,
            details.presentModes);
    }

    return details;
}

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, First_app* app) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(app->physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
        return i;
        }
    }
    return 0;
}

void createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer *buffer,
    VkDeviceMemory *bufferMemory,
    First_app* app) {

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(app->device_, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
        return;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(app->device_, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, app);

    if (vkAllocateMemory(app->device_, &allocInfo, NULL, bufferMemory) != VK_SUCCESS) {
        return;
    }

    vkBindBufferMemory(app->device_, *buffer, *bufferMemory, 0);
}

VkCommandBuffer beginSingleTimeCommands(First_app* app) {
  VkCommandBufferAllocateInfo allocInfo;
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = app->commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(app->device_, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  return commandBuffer;
}

void endSingleTimeCommands(VkCommandBuffer commandBuffer, First_app* app) {
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo;
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(app->graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(app->graphicsQueue_);

  vkFreeCommandBuffers(app->device_, app->commandPool, 1, &commandBuffer);
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, First_app* app) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands(app);

  VkBufferCopy copyRegion;
  copyRegion.srcOffset = 0;  // Optional
  copyRegion.dstOffset = 0;  // Optional
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  endSingleTimeCommands(commandBuffer, app);
}

void copyBufferToImage(
    VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount, First_app* app) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(app);

    VkBufferImageCopy region = { .imageOffset = {0, 0, 0}, .imageExtent = {width, height, 1} };
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);
    endSingleTimeCommands(commandBuffer, app);
}

void createImageWithInfo(
    const VkImageCreateInfo *imageInfo,
    VkMemoryPropertyFlags properties,
    VkImage *image,
    VkDeviceMemory *imageMemory,
    First_app* app) {

    if (vkCreateImage(app->device_, imageInfo, NULL, image) != VK_SUCCESS) {
        return;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(app->device_, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, app);

    if (vkAllocateMemory(app->device_, &allocInfo, NULL, imageMemory) != VK_SUCCESS) {
        return;
    }

    if (vkBindImageMemory(app->device_, *image, *imageMemory, 0) != VK_SUCCESS) {
        return;
    }
}
