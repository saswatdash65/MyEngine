#pragma once
#include "core_app.h"
#include "sdlwindow.h"

char* readShader(const char* filePath);

// Buffer helpers
void createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer *buffer,
    VkDeviceMemory *bufferMemory,
    First_app* app);
VkCommandBuffer beginSingleTimeCommands(First_app* app);
void endSingleTimeCommands(VkCommandBuffer commandBuffer, First_app* app);
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, First_app* app);
void copyBufferToImage(
    VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount, First_app* app);

void createImageWithInfo(
    const VkImageCreateInfo *imageInfo,
    VkMemoryPropertyFlags properties,
    VkImage *image,
    VkDeviceMemory *imageMemory,
    First_app* app);

// Vulkan Helpers
int isDeviceSuitable(VkPhysicalDevice device, First_app* app);
const char ** getRequiredExtensions(uint32_t* extensionCount, sdlWIndow* app);
int checkValidationLayerSupport(First_app* app);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, First_app* app);
#ifndef NDEBUG
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *createInfo);
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger);
#endif
int checkDeviceExtensionSupport(VkPhysicalDevice device, First_app* app);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, First_app* app);
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, First_app* app);
