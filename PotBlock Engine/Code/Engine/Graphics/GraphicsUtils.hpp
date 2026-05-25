#pragma once

// These headers should be first in the file to avoid false implementations
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include "ThirdParty/glslang/Include/glslang_c_interface.h"
//
#include "Engine/Graphics/GPUBuffer.hpp"
#include "Engine/Graphics/QueueFamilyIndices.hpp"
#include "Engine/Graphics/SwapchainSupportDetails.hpp"
#include "Engine/Graphics/Mesh.hpp"

#include "Engine/Graphics/Window.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <cassert>

namespace PBE
{
	constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	enum class DescriptorSetType
	{
		RENDER = 0,
		MESH = 3,
		MATERIAL = 2,
		ANIMATION = 1,
		COUNT
	};

	enum class SamplerType
	{
		NEAREST_REPEAT,
		NEAREST_CLAMP,
		LINEAR_REPEAT,
		LINEAR_CLAMP,
		COUNT
	};

	std::vector<uint32_t> CompileGLSLToSPIRV(glslang_stage_t stage, std::filesystem::path path);
	//Checks if a Vulkan function returns an error
	void VkErrorCheck(VkResult result, std::string_view message = "");
	void VkLabelObject(VkDevice device, VkObjectType objectType, void* objectHandle, std::string_view objectName);
	void VkTagObject(VkDevice device, VkObjectType objectType, void* objectHandle, uint64_t tagName, void* tag, size_t tagSize);
	void VkBeginDebugUtilsLabel(VkDevice device, VkCommandBuffer commandBuffer, std::string_view labelName, float color[4]);
	void VkEndDebugUtilsLabel(VkDevice device, VkCommandBuffer commandBuffer);

	//Instance Creation

	//Creates a Vulkan instance
	bool CheckValidationLayerSupport();
	std::vector<char const*> GetRequiredExtensions();
	VkInstance CreateInstance(std::string_view appName, std::string_view engineName);
	VkDebugUtilsMessengerEXT CreateDebugMessenger(VkInstance instance);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	//Logical Device Creation
	
	//Check device extension support
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	//Rates a physical device based on its capabilities
	int RateDeviceSuitability(VkPhysicalDevice device);
	//Picks a physical device with reasonable capabilities
	VkPhysicalDevice PickPhysicalDevice(VkInstance instance);

	//Finds the queue families that are supported by the physical device
	QueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	VkDevice CreateLogicalDevice(
		VkPhysicalDevice physicalDevice,
		VkSurfaceKHR surface,
		uint32_t* graphicsQueueIndex,
		VkQueue* graphicsQueue,
		uint32_t* presentQueueIndex,
		VkQueue* presentQueue,
		uint32_t* computeQueueIndex,
		VkQueue* computeQueue
	);
	
	//Surface Creation
	VkSurfaceKHR CreateSurface(VkInstance instance, Window* window);

	//Memory Allocators
	//Creates a Vulkan Memory Allocator instance. Should in theory be called once per application.
	VmaAllocator CreateAllocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);

	void VmaShowDebugStats(VmaAllocator allocator);
	//Buffer Creation. Allocates memory for a buffer on the GPU.
	GPUBuffer CreateGPUBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	//Buffer Destruction. Frees the memory used by the buffer into the allocator it was created from.
	void DestroyGPUBuffer(VmaAllocator allocator, GPUBuffer buffer);

	//Buffer Copying. Copies the contents of one buffer to another then waits for the operation to finish.
	void SingleTimeCopyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue, GPUBuffer srcBuffer, GPUBuffer dstBuffer, VkDeviceSize size);
	//Buffer Copying. Copies the contents of one buffer to another then waits for the operation to finish.
	void SingleTimeCopyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	void AppendCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	//Swapchain Creation
	
	//Get swapchain support details
	SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

	//Gets the best surface format for the window
	VkSurfaceFormatKHR GetSwapchainSurfaceFormat(VkPhysicalDevice, VkSurfaceKHR surface);
	//Gets the best surface format for the window
	VkSurfaceFormatKHR GetSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	//Gets the best presentation mode for the window
	VkPresentModeKHR GetSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	//Gets the best extent for the window
	VkExtent2D GetSwapchainExtent(const VkSurfaceCapabilitiesKHR& m_Capabilities, VkExtent2D windowExtent);
	//Creates a swapchain for the window
	VkSwapchainKHR CreateSwapchain(
		VkPhysicalDevice physicalDevice, 
		VkDevice device, 
		VkSurfaceKHR surface, 
		VkExtent2D windowExtent, 
		uint32_t graphicsQueueIndex,
		uint32_t presentQueueIndex,
		uint32_t computeQueueIndex,
		VkFormat* out_swapchainImageFormat,
		VkExtent2D* out_swapchainExtent,
		uint32_t* out_imageCount,
		VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE
	);

	VkImage* GetSwapchainImages(VkDevice device, VkSwapchainKHR swapchain, uint32_t* imageCount);
	std::vector<VkImageView> CreateSwapchainImageViews(VkDevice device, VkImage* images, uint32_t imageCount, VkFormat format);

	//VkRenderPass CreateRenderPass(VkDevice device, VkFormat imageFormat);
	VkCommandPool CreateCommandPool(VkDevice device, VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex);
	std::vector<VkCommandBuffer> CreateCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t count);
	VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
	void EndSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer);

	std::vector<VkSemaphore> CreateSemaphores(VkDevice device, VkSemaphoreCreateFlags flags, uint32_t count);
	std::vector<VkFence> CreateFences(VkDevice device, VkFenceCreateFlags flags, uint32_t count);
	
	VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits maxSamples = VK_SAMPLE_COUNT_64_BIT);

	//Creates a shader module from a vert or frag shader file. Shader names should always be unique.
	VkShaderModule CreateOrGetShaderModule(VkDevice device, glslang_stage_t stage, std::filesystem::path path);
	VkShaderModule CreateOrGetShaderModule(VkDevice device, std::vector<uint32_t> const& code);

	template <typename T_ValueType>
	VkVertexInputBindingDescription CreateVertexInputBindingDescription(uint32_t binding, std::vector<T_ValueType> const& vertBuffer, VkVertexInputRate rate = VK_VERTEX_INPUT_RATE_VERTEX)
	{
		VkVertexInputBindingDescription bindingDescription
		{
			.binding = binding,
			.stride = sizeof(T_ValueType),
			.inputRate = rate
		};

		return bindingDescription;
	}

	VkImage CreateTexture2D(VmaAllocator allocator, VkDevice device, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage);
	// 
	VkPipeline CreateGraphicsPipeline(
		VkDevice device, 
		VkPipelineLayout layout, 
		VkPipelineVertexInputStateCreateInfo vertexInputInfo,
		VkSampleCountFlagBits samples,
		VkRenderPass renderPass, 
		uint32_t subpassIndex, 
		VkExtent2D extent, 
		VkShaderModule vertShader, 
		VkShaderModule fragShader,
		bool depthTestEnabled = true
	);
	VkPipeline CreateGraphicsPipeline(VkDevice device, VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpassIndex, VkExtent2D extent, VkShaderModule vertShader, VkShaderModule fragShader, VkShaderModule geomShader);
	// 
	VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding> const& bindings);

	VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice);

	bool HasStencilComponent(VkFormat format);
	void TransitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void CopyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer, VkDeviceSize bufferSize, VkImage image, uint32_t width, uint32_t height);
	void GenerateMipmaps(
		VkDevice device,
		VkCommandPool pool,
		VkQueue queue,
		VkImage image,
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels
	);
}