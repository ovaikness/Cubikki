#include "Engine/Graphics/GraphicsUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/IO/FileUtils.hpp"
#include "Engine/Graphics/Mesh.hpp"

#include <array>
#include <iostream>
#include <set>
#include <string>
#include <string_view>

// Required for use of glslang_default_resource
#include "ThirdParty/glslang/Public/resource_limits_c.h"

std::vector<char const*> const validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

std::vector<char const*> const deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

constexpr bool enableValidationLayers = false;
constexpr bool enableDebugMarkers     = false;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* /*pUserData*/) {

	std::cerr << "[Vulkan Debug] : " << pCallbackData->pMessage << std::endl;
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		__debugbreak();
	}

	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void PBE::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void VkDebugMarkerObjectNameEXT(VkDevice device, VkDebugUtilsObjectNameInfoEXT const& nameInfo)
{
	auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
	if (func != nullptr)
	{
		func(device, &nameInfo);
	}
}

void VkDebugSetObjectTagEXT(VkDevice device, VkDebugUtilsObjectTagInfoEXT const& tagInfo)
{
	auto func = (PFN_vkSetDebugUtilsObjectTagEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectTagEXT");
	if (func != nullptr)
	{
		func(device, &tagInfo);
	}
}

std::vector<uint32_t> PBE::CompileGLSLToSPIRV(glslang_stage_t stage, std::filesystem::path path)
{
	std::vector<uint8_t> shaderSource = PBE::ReadFileToBuffer(path);
	shaderSource.push_back('\0');
	const glslang_input_t input = {
	   .language = GLSLANG_SOURCE_GLSL,
	   .stage = stage,
	   .client = GLSLANG_CLIENT_VULKAN,
	   .client_version = GLSLANG_TARGET_VULKAN_1_2,
	   .target_language = GLSLANG_TARGET_SPV,
	   .target_language_version = GLSLANG_TARGET_SPV_1_3,
	   .code = (const char*)shaderSource.data(),
	   .default_version = 100,
	   .default_profile = GLSLANG_NO_PROFILE,
	   .force_default_version_and_profile = false,
	   .forward_compatible = false,
	   .messages = GLSLANG_MSG_DEFAULT_BIT,
	   .resource = glslang_default_resource(),
	};

	glslang_shader_t* shader = glslang_shader_create(&input);

	std::vector<uint32_t> binary;
	if (!glslang_shader_preprocess(shader, &input)) {
		printf("Preprocess Error Log: %s\n", glslang_shader_get_info_log(shader));
		printf("Preprocess Debug Log: %s\n", glslang_shader_get_info_debug_log(shader));
		throw std::runtime_error("GLSL preprocessing failed");
	}

	if (!glslang_shader_parse(shader, &input)) {
		printf("Parse Error Log: %s\n", glslang_shader_get_info_log(shader));
		printf("Parse Debug Log: %s\n", glslang_shader_get_info_debug_log(shader));
		throw std::runtime_error("GLSL preprocessing failed");
	}

	glslang_program_t* program = glslang_program_create();
	glslang_program_add_shader(program, shader);

	if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
		throw std::runtime_error("GLSL preprocessing failed");
	}

	glslang_program_SPIRV_generate(program, stage);

	binary.resize(glslang_program_SPIRV_get_size(program));
	glslang_program_SPIRV_get(program, binary.data());

	const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
	if (spirv_messages)
		printf("(%s) %s\b", path.string().c_str(), spirv_messages);

	glslang_program_delete(program);
	glslang_shader_delete(shader);

	return binary;
}

void PBE::VkErrorCheck(VkResult result, std::string_view message)
{
	assert(result == VK_SUCCESS && message.data());
	if (result != VK_SUCCESS)
	{
		std::cout << std::string("Vulkan Error : ") + message.data() << std::endl;
		__debugbreak();
		//throw std::runtime_error(std::string("Vulkan Error : ") + message.data());
	}
}

void PBE::VkLabelObject(VkDevice device, VkObjectType objectType, void* objectHandle, std::string_view objectName)
{
	if (!enableDebugMarkers)
	{
		return;
	}

	VkDebugUtilsObjectNameInfoEXT nameInfo{};
	nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	nameInfo.objectType = objectType;
	nameInfo.objectHandle = reinterpret_cast<uint64_t>(objectHandle);
	nameInfo.pObjectName = objectName.data();

	VkDebugMarkerObjectNameEXT(device, nameInfo);
}

void PBE::VkTagObject(VkDevice device, VkObjectType objectType, void* objectHandle, uint64_t tagName, void* tag, size_t tagSize)
{
	if (!enableDebugMarkers)
	{
		return;
	}
	VkDebugUtilsObjectTagInfoEXT tagInfo{};
	tagInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT;
	tagInfo.objectType = objectType;
	tagInfo.objectHandle = reinterpret_cast<uint64_t>(objectHandle);
	tagInfo.tagName = tagName;
	tagInfo.tagSize = tagSize;
	tagInfo.pTag = tag;

	VkDebugSetObjectTagEXT(device, tagInfo);
}

void PBE::VkBeginDebugUtilsLabel(VkDevice device, VkCommandBuffer commandBuffer, std::string_view labelName, float color[4])
{
	if (!enableDebugMarkers)
	{
		return;
	}

	static auto func = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkCmdBeginDebugUtilsLabelEXT");

	VkDebugUtilsLabelEXT labelInfo{};
	labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	labelInfo.pLabelName = labelName.data();
	memcpy(labelInfo.color, color, sizeof(float) * 4);

	func(commandBuffer, &labelInfo);
}

void PBE::VkEndDebugUtilsLabel(VkDevice device, VkCommandBuffer commandBuffer)
{
	if (!enableDebugMarkers)
	{
		return;
	}

	static auto func = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkCmdEndDebugUtilsLabelEXT");

	func(commandBuffer);
}

bool PBE::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (char const* layerName : validationLayers)
	{
		bool layerFound = false;

		for (VkLayerProperties const& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

std::vector<char const*> PBE::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	char const** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<char const*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

VkInstance PBE::CreateInstance(std::string_view appName, std::string_view engineName)
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = appName.data();
	appInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
	appInfo.pEngineName = engineName.data();
	appInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	std::vector<char const*> extensions = GetRequiredExtensions();

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

	if constexpr (enableValidationLayers && !CheckValidationLayerSupport())
	{
		//PBE::DevConsole::ErrorDie("Validation layers requested, but not available!");
	}

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = debugCallback;
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	VkInstance instance;
	VkErrorCheck(vkCreateInstance(&createInfo, nullptr, &instance));

	return instance;
}

VkDebugUtilsMessengerEXT PBE::CreateDebugMessenger(VkInstance instance)
{
	if (!enableValidationLayers)
	{
		return VK_NULL_HANDLE;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;

	VkDebugUtilsMessengerEXT debugMessenger;
	VkErrorCheck(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger));

	return debugMessenger;
}

bool PBE::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (VkExtensionProperties const& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

int PBE::RateDeviceSuitability(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	int score = 0;

	if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1000;
	}

	score += deviceProperties.limits.maxImageDimension2D;

	if (!deviceFeatures.geometryShader)
	{
		return 0;
	}

	return score;
}

VkPhysicalDevice PBE::PickPhysicalDevice(VkInstance instance)
{
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	uint32_t deviceCount = 0;

	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		return VK_NULL_HANDLE;
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	int bestScore = 0;
	for (const auto& device : devices)
	{
		if (RateDeviceSuitability(device) > bestScore)
		{
			physicalDevice = device;
			bestScore = RateDeviceSuitability(device);
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		physicalDevice = devices[0];
	}

	return physicalDevice;
}

PBE::QueueFamilyIndices PBE::FindQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;

	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}
		
		if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			indices.computeFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.IsComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

VkDevice PBE::CreateLogicalDevice(
	VkPhysicalDevice physicalDevice, 
	VkSurfaceKHR surface, 
	uint32_t* graphicsQueueIndex,
	VkQueue* graphicsQueue, 
	uint32_t* presentQueueIndex,
	VkQueue* presentQueue, 
	uint32_t* computeQueueIndex,
	VkQueue* computeQueue)
{
	VkDevice device = VK_NULL_HANDLE;

	std::vector<VkDeviceQueueCreateInfo> queueFamilyCreateInfos;

	QueueFamilyIndices indices = FindQueueFamilyIndices(physicalDevice, surface);

	float queuePriority = 1.0f;

	if (indices.graphicsFamily.has_value())
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueFamilyCreateInfos.push_back(queueCreateInfo);
	}

	if (indices.presentFamily.has_value() && indices.presentFamily != indices.graphicsFamily)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.presentFamily.value();
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueFamilyCreateInfos.push_back(queueCreateInfo);
	}

	if (indices.computeFamily.has_value() && indices.graphicsFamily != indices.presentFamily && indices.computeFamily != indices.graphicsFamily)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.computeFamily.value();
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueFamilyCreateInfos.push_back(queueCreateInfo);
	}


	VkPhysicalDeviceFeatures features{};
	features.geometryShader = VK_TRUE;
	features.samplerAnisotropy = VK_TRUE;
	features.sampleRateShading = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueFamilyCreateInfos.data();
	createInfo.queueCreateInfoCount = (uint32_t)queueFamilyCreateInfos.size();
	createInfo.pEnabledFeatures = &features;
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());

	VkErrorCheck(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device));

	graphicsQueueIndex = &indices.graphicsFamily.value();
	presentQueueIndex  = &indices.presentFamily.value();
	computeQueueIndex  = &indices.computeFamily.value();

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, presentQueue);
	vkGetDeviceQueue(device, indices.computeFamily.value(), 0, computeQueue);

	return device;
}

VkSurfaceKHR PBE::CreateSurface(VkInstance instance, Window* window)
{
	VkSurfaceKHR surface = VK_NULL_HANDLE;

#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hinstance = GetModuleHandle(nullptr);
	createInfo.hwnd = glfwGetWin32Window(window->GetWindow());
	VkErrorCheck(vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface));
#endif
	return surface;
}

VmaAllocator PBE::CreateAllocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
{
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;

	VmaAllocator allocator;
	VkErrorCheck(vmaCreateAllocator(&allocatorInfo, &allocator));

	return allocator;
}

void PBE::VmaShowDebugStats(VmaAllocator allocator)
{
	char* statsString;
	vmaBuildStatsString(allocator, &statsString, VK_TRUE);
	printf("%s\n", statsString);
	vmaFreeStatsString(allocator, statsString);
}

PBE::GPUBuffer PBE::CreateGPUBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = memoryUsage;

	GPUBuffer buffer;
	buffer.m_Size = size;

	VkErrorCheck(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer.m_Buffer, &buffer.m_Allocation,nullptr));
	static int bufferNameCounter = 0;

	vmaSetAllocationName(allocator, buffer.m_Allocation, std::format("GPU Buffer {}", bufferNameCounter++).c_str());

	return buffer;
}

void PBE::DestroyGPUBuffer(VmaAllocator allocator, GPUBuffer buffer)
{
	if (buffer.m_Data)
	{
		vmaUnmapMemory(allocator, buffer.m_Allocation);
	}
	vmaDestroyBuffer(allocator, buffer.m_Buffer, buffer.m_Allocation);

	buffer.m_Buffer = VK_NULL_HANDLE;
	buffer.m_Allocation = VK_NULL_HANDLE;
}

void PBE::SingleTimeCopyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue, GPUBuffer srcBuffer, GPUBuffer dstBuffer, VkDeviceSize size)
{
	SingleTimeCopyBuffer(device, commandPool, queue, srcBuffer.m_Buffer, dstBuffer.m_Buffer, size);
}

void PBE::SingleTimeCopyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void PBE::AppendCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	VkBufferMemoryBarrier bufferBarrier{};
	bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	bufferBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	bufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferBarrier.buffer = dstBuffer;
	bufferBarrier.offset = 0;
	bufferBarrier.size = size;
	
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &bufferBarrier, 0, nullptr);
}

PBE::SwapchainSupportDetails PBE::QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	uint32_t formatCount;
	uint32_t presentModeCount;

	SwapchainSupportDetails details;
	 
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.m_Capabilities);

	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.m_Formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.m_Formats.data());
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.m_PresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.m_PresentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR PBE::GetSwapchainSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	auto details = QuerySwapchainSupport(physicalDevice, surface);
	for (const auto& availableFormat : details.m_Formats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}
	//Could not find available swapchain format
	__debugbreak();
	return details.m_Formats[0];
}

VkSurfaceFormatKHR PBE::GetSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR PBE::GetSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D PBE::GetSwapchainExtent(const VkSurfaceCapabilitiesKHR& m_Capabilities, VkExtent2D windowExtent)
{
	if (m_Capabilities.currentExtent.width != UINT32_MAX)
	{
		return m_Capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = windowExtent;
		actualExtent.width = Max(m_Capabilities.minImageExtent.width, Min(m_Capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = Max(m_Capabilities.minImageExtent.height, Min(m_Capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;

	}
	return { 0,0 };
}

VkSwapchainKHR PBE::CreateSwapchain(
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
	VkSwapchainKHR oldSwapchain
)
{
	SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(physicalDevice, surface);
	VkSurfaceFormatKHR surfaceFormat = GetSwapchainSurfaceFormat(swapchainSupport.m_Formats);
	VkPresentModeKHR presentMode = GetSwapchainPresentMode(swapchainSupport.m_PresentModes);
	VkExtent2D extent = GetSwapchainExtent(swapchainSupport.m_Capabilities, windowExtent);

	uint32_t imageCount = swapchainSupport.m_Capabilities.minImageCount + 1;
	if (swapchainSupport.m_Capabilities.maxImageCount > 0 && imageCount > swapchainSupport.m_Capabilities.maxImageCount)
	{
		imageCount = swapchainSupport.m_Capabilities.maxImageCount;
	}

	if (out_imageCount)
	{
		*out_imageCount = imageCount;
	}

	if (out_swapchainImageFormat)
	{
		*out_swapchainImageFormat = surfaceFormat.format;
	}

	if (out_swapchainExtent)
	{
		*out_swapchainExtent = extent;
	}

	VkSwapchainCreateInfoKHR createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	std::vector<uint32_t> queueFamilyIndices;
	queueFamilyIndices.push_back(graphicsQueueIndex);
	if (std::find(queueFamilyIndices.begin(), queueFamilyIndices.end(), presentQueueIndex) == queueFamilyIndices.end())
	{
		queueFamilyIndices.push_back(presentQueueIndex);
	}
	if (std::find(queueFamilyIndices.begin(), queueFamilyIndices.end(), computeQueueIndex) == queueFamilyIndices.end())
	{
		queueFamilyIndices.push_back(computeQueueIndex);
	}

	if (queueFamilyIndices.size() == 1)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = (uint32_t)queueFamilyIndices.size();
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}

	createInfo.preTransform = swapchainSupport.m_Capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = oldSwapchain;

	VkSwapchainKHR swapchain;
	VkErrorCheck(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain));

	return swapchain;
}

VkImage* PBE::GetSwapchainImages(VkDevice device, VkSwapchainKHR swapchain, uint32_t* imageCount)
{
	vkGetSwapchainImagesKHR(device, swapchain, imageCount, nullptr);
	VkImage* images = new VkImage[*imageCount];
	vkGetSwapchainImagesKHR(device, swapchain, imageCount, images);
	return images;
}

std::vector<VkImageView> PBE::CreateSwapchainImageViews(VkDevice device, VkImage* images, uint32_t imageCount, VkFormat format)
{
	std::vector<VkImageView> imageViews(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkErrorCheck(vkCreateImageView(device, &createInfo, nullptr, &imageViews[i]));
	}

	return imageViews;
}

VkCommandPool PBE::CreateCommandPool(VkDevice device, VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex)
{
	VkCommandPool commandPool;
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndex;
	poolInfo.flags = flags;
	
	VkErrorCheck(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));

	return commandPool;
}

std::vector<VkCommandBuffer> PBE::CreateCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t count)
{
	std::vector<VkCommandBuffer> commandBuffers(count);
	for (uint32_t i = 0; i < count; ++i)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkErrorCheck(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffers[i]));
	}

	return commandBuffers;
}

VkCommandBuffer PBE::BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	VkErrorCheck(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkErrorCheck(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	return commandBuffer;
}

void PBE::EndSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer)
{
	VkErrorCheck(vkEndCommandBuffer(commandBuffer));
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	VkErrorCheck(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
	VkErrorCheck(vkQueueWaitIdle(queue));
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

std::vector<VkSemaphore> PBE::CreateSemaphores(VkDevice device, VkSemaphoreCreateFlags flags, uint32_t count)
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreInfo.flags = flags;

	std::vector<VkSemaphore> semaphores(count);

	for (uint32_t i = 0; i < count; ++i)
	{
		VkErrorCheck(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphores[i]));
	}

	return semaphores;
}

std::vector<VkFence> PBE::CreateFences(VkDevice device, VkFenceCreateFlags flags, uint32_t count)
{
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = flags;

	std::vector<VkFence> fences(count);

	for (uint32_t i = 0; i < count; ++i)
	{
		VkErrorCheck(vkCreateFence(device, &fenceInfo, nullptr, &fences[i]));
	}

	return fences;
}

VkSampleCountFlagBits PBE::GetMaxUsableSampleCount(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits maxSamples)
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT && counts <= maxSamples) {
		return VK_SAMPLE_COUNT_64_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_32_BIT && counts <= maxSamples) {
		return VK_SAMPLE_COUNT_32_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_16_BIT && counts <= maxSamples) {
		return VK_SAMPLE_COUNT_16_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_8_BIT && counts <= maxSamples) {
		return VK_SAMPLE_COUNT_8_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_4_BIT && counts <= maxSamples) {
		return VK_SAMPLE_COUNT_4_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_2_BIT && counts <= maxSamples) {
		return VK_SAMPLE_COUNT_2_BIT;
	}
	return VK_SAMPLE_COUNT_1_BIT;
}

VkShaderModule PBE::CreateOrGetShaderModule(VkDevice device, glslang_stage_t stage, std::filesystem::path path)
{
	std::vector<uint32_t> code;
	const std::filesystem::path cachePath = "Temporary/CompiledShaders/";
	std::string cacheFileStr = path.filename().string() + ".spv";
	std::filesystem::path cacheFile = cachePath / cacheFileStr;
	if (std::filesystem::exists(cacheFileStr))
	{
		auto writeTimeCache = std::filesystem::last_write_time(cacheFile);
		auto writeTimeSource = std::filesystem::last_write_time(path);

		if (writeTimeCache < writeTimeSource)
		{
			code = CompileGLSLToSPIRV(stage, path);
			PBE::WriteUint32BufferToFile(cacheFile, code);
		}
		else
		{
			code = PBE::ReadFileToUint32Buffer(cacheFile);
		}
	}
	else
	{
		code = CompileGLSLToSPIRV(stage, path);
		PBE::WriteUint32BufferToFile(cacheFile, code);
	}

	VkShaderModuleCreateInfo createInfo
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size() * sizeof(uint32_t),
		.pCode = code.data()
	};

	VkShaderModule shaderModule;
	VkErrorCheck(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
	VkLabelObject(device, VK_OBJECT_TYPE_SHADER_MODULE, shaderModule, path.filename().string());
	return shaderModule;
}

VkShaderModule PBE::CreateOrGetShaderModule(VkDevice device, std::vector<uint32_t> const& code)
{
	VkShaderModuleCreateInfo createInfo
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size() * sizeof(uint32_t),
		.pCode = code.data()
	};

	VkShaderModule shaderModule;
	VkErrorCheck(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
	VkLabelObject(device, VK_OBJECT_TYPE_SHADER_MODULE, shaderModule, "Custom Data Shader");
	return shaderModule;
}

VkImage PBE::CreateTexture2D(VmaAllocator allocator, VkDevice /*device*/, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	VkImageCreateInfo imageInfo
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = { extent.width, extent.height, 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.tiling = tiling,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VmaAllocationCreateInfo allocInfo
	{
		.usage = memoryUsage
	};

	VkImage image;
	VmaAllocation allocation;

	VkErrorCheck(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr));
	static int textureCounter = 0;
	vmaSetAllocationName(allocator, allocation, std::format("Texture 2D Allocation {}", textureCounter++).c_str());

	return image;
}

VkPipeline PBE::CreateGraphicsPipeline(VkDevice device, VkPipelineLayout layout, VkPipelineVertexInputStateCreateInfo vertexInputInfo, VkSampleCountFlagBits samples, VkRenderPass renderPass, uint32_t subpassIndex, VkExtent2D extent, VkShaderModule vertShader, VkShaderModule fragShader, bool depthTestEnabled /* = true*/)
{
	VkPipelineShaderStageCreateInfo vertShaderStageInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertShader,
		.pName = "main"
	};

	VkPipelineShaderStageCreateInfo fragShaderStageInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragShader,
		.pName = "main"
	};

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineInputAssemblyStateCreateInfo inputAssembly
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkViewport viewport
	{
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(extent.width),
		.height = static_cast<float>(extent.height)
	};

	VkRect2D scissor
	{
		.offset = { 0, 0 },
		.extent = extent
	};

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_TRUE;
	multisampling.rasterizationSamples = samples;
	multisampling.minSampleShading = 0.8f;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO; 
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	if (!depthTestEnabled)
	{
		depthStencil.depthTestEnable = VK_FALSE;
		depthStencil.depthWriteEnable = VK_FALSE;
	}

	colorBlending.pAttachments = &colorBlendAttachment;

	std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH, VK_DYNAMIC_STATE_SCISSOR };
	
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 3;
	dynamicState.pDynamicStates = dynamicStates.data();

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = layout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = subpassIndex;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline pipeline;
	VkErrorCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));

	return pipeline;
}

VkPipeline PBE::CreateGraphicsPipeline(VkDevice device, VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpassIndex, VkExtent2D extent, VkShaderModule vertShader, VkShaderModule fragShader, VkShaderModule geomShader)
{
	VkPipelineShaderStageCreateInfo vertShaderStageInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertShader,
		.pName = "main"
	};

	VkPipelineShaderStageCreateInfo fragShaderStageInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragShader,
		.pName = "main"
	};

	VkPipelineShaderStageCreateInfo geomShaderStageInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_GEOMETRY_BIT,
		.module = geomShader,
		.pName = "main"
	};

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo, geomShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkViewport viewport
	{
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(extent.width),
		.height = static_cast<float>(extent.height)
	};

	VkRect2D scissor
	{
		.offset = { 0, 0 },
		.extent = extent
	};

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;

	colorBlending.pAttachments = &colorBlendAttachment;

	std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 3;
	dynamicState.pDynamicStates = dynamicStates.data();

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	pipelineInfo.stageCount = 3;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = layout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = subpassIndex;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline pipeline;
	VkErrorCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));

	return pipeline;
}

VkDescriptorSetLayout PBE::CreateDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding> const& bindings )
{
	VkDescriptorSetLayoutCreateInfo layoutInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data()
	};

	VkDescriptorSetLayout layout;
	VkErrorCheck(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout));

	return layout;
}

VkFormat PBE::FindDepthFormat(VkPhysicalDevice /*physicalDevice*/)
{
	return VK_FORMAT_D24_UNORM_S8_UINT;
}

bool PBE::HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void PBE::TransitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (HasStencilComponent(format))
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage = 0;
	VkPipelineStageFlags destinationStage = 0;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		//PBE::DevConsole::ErrorDie("Unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0, // No flags
		0, nullptr, // No memory barriers
		0, nullptr, // No buffer memory barriers
		1, &barrier // Image memory barrier
	);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void PBE::CopyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer, VkDeviceSize /*bufferSize*/, VkImage image, uint32_t width, uint32_t height)
{
	// Create a command buffer for the copy operation
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	// Begin recording the command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	// Specify the region of the buffer to copy from
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0; // Specifies tightly packed buffer, as 0 means "tightly packed"
	region.bufferImageHeight = 0; // Same as above, 0 for tightly packed

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 }; // Copy starts from (0, 0)
	region.imageExtent = { width, height, 1 }; // The size of the image in pixels

	// Execute the copy command
	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,                 // The source buffer
		image,                  // The destination image
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // The layout of the destination image
		1,                      // Number of regions to copy
		&region                 // Region details
	);

	// End recording the command buffer
	vkEndCommandBuffer(commandBuffer);

	// Submit the command buffer for execution
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	// Clean up
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void PBE::GenerateMipmaps(VkDevice device, VkCommandPool pool, VkQueue queue, VkImage image, uint32_t width, uint32_t height, uint32_t mipLevels)
{
	// Create a command buffer for the copy operation
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	// Begin recording the command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	int32_t mipWidth = width;
	int32_t mipHeight = height;

	for (uint32_t i = 1; i < mipLevels; i++) {
		// Define the regions for the blit operation
		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;

		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		// Reduce dimensions for next mip level
		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	// End recording the command buffer
	vkEndCommandBuffer(commandBuffer);

	// Submit the command buffer for execution
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);
}