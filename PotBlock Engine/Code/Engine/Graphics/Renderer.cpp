#include "Engine/Graphics/Renderer.hpp"
#include "Engine/Graphics/GraphicsUtils.hpp"
#include "Engine/Graphics/Texture.hpp"
#include "Engine/Graphics/Material.hpp"
#include "Engine/Graphics/Mesh.hpp"
#include "Engine/Graphics/Camera.hpp"
#include "Engine/Graphics/Animation.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Graphics/UI/Widget.hpp"

#include <iostream>
#include <stb/stb_image.h>
#include <array>
#include <stdio.h>
#include <stdlib.h>

#include "imgui_impl_vulkan.h"

PBE::Renderer::Renderer(RendererCreateInfo const& info)
	: m_Info(info)
{
	AABB2 view;
	view.m_Mins = Vec2(0, 0);
	view.m_Maxs = Vec2(1.f, 1.f);

	//m_DebugUICamera.CreateOrthographic(view, 0.f, 1000.f);

	m_Instance		 = CreateInstance(info.ApplicationName, info.EngineName);
	m_DebugMessenger = CreateDebugMessenger(m_Instance);
	m_PhysicalDevice = PickPhysicalDevice(m_Instance);
	m_MsaaSamples	 = GetMaxUsableSampleCount(m_PhysicalDevice, VK_SAMPLE_COUNT_16_BIT);

	assert(info.m_pWindow != nullptr);
	m_Surface = CreateSurface(m_Instance, info.m_pWindow);
	m_Device = CreateLogicalDevice(m_PhysicalDevice, m_Surface,
		&m_GraphicsQueueIndex,
		&m_GraphicsQueue,
		&m_PresentQueueIndex,
		&m_PresentQueue,
		&m_ComputeQueueIndex,
		&m_ComputeQueue
	);
	
	//Label Queue
	std::string graphicsQueueLabel = "Graphics Queue";
	std::string presentQueueLabel = "Present Queue";
	std::string computeQueueLabel = "Compute Queue";

	if (m_GraphicsQueue == m_PresentQueue)
	{
		graphicsQueueLabel += " | Present Queue";
	}

	if (m_GraphicsQueue == m_ComputeQueue)
	{
		graphicsQueueLabel += " | Compute Queue";
	}

	if (m_PresentQueue == m_ComputeQueue)
	{
		presentQueueLabel += " | Compute Queue";
	}


	VkLabelObject(m_Device, VK_OBJECT_TYPE_QUEUE, m_GraphicsQueue, graphicsQueueLabel);

	if (m_GraphicsQueue != m_PresentQueue)
	{
		VkLabelObject(m_Device, VK_OBJECT_TYPE_QUEUE, m_PresentQueue, presentQueueLabel);
	}

	if (m_GraphicsQueue != m_ComputeQueue && m_PresentQueue != m_ComputeQueue)
	{
		VkLabelObject(m_Device, VK_OBJECT_TYPE_QUEUE, m_ComputeQueue, computeQueueLabel);
	}

	m_CommandPool	 = CreateCommandPool(m_Device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, m_GraphicsQueueIndex);
	m_CommandBuffers = CreateCommandBuffers(m_Device, m_CommandPool, MAX_FRAMES_IN_FLIGHT);

	m_ImageAvailableSemaphore = CreateSemaphores(m_Device, 0, MAX_FRAMES_IN_FLIGHT);
	m_RenderFinishedSemaphore = CreateSemaphores(m_Device, 0, MAX_FRAMES_IN_FLIGHT);
	m_InFlightFences		  = CreateFences(m_Device, VK_FENCE_CREATE_SIGNALED_BIT, MAX_FRAMES_IN_FLIGHT);
	
	CreateTransferFence();
	CreatePostProcessRenderPass();
	CreateSwapchainResources();

	VmaAllocatorCreateInfo allocatorInfo
	{
		.physicalDevice = m_PhysicalDevice,
		.device = m_Device,
		.instance = m_Instance,
		.vulkanApiVersion = VK_API_VERSION_1_3,
	};
	vmaCreateAllocator(&allocatorInfo, &m_Allocator);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		std::string indexLabel = std::to_string(i);

		VkLabelObject(m_Device, VK_OBJECT_TYPE_SEMAPHORE, m_ImageAvailableSemaphore[i], std::string("Image Available Semaphore") + indexLabel);
		VkLabelObject(m_Device, VK_OBJECT_TYPE_SEMAPHORE, m_RenderFinishedSemaphore[i], std::string("Render Finished Semaphore") + indexLabel);
		VkLabelObject(m_Device, VK_OBJECT_TYPE_FENCE, m_InFlightFences[i], std::string("In Flight Fence") + indexLabel);
	}

	VkLabelObject(m_Device, VK_OBJECT_TYPE_COMMAND_POOL, m_CommandPool, "Graphics Command Pool");

	for (int i = 0; i< m_CommandBuffers.size(); ++ i)
	{
		VkCommandBuffer buff = m_CommandBuffers[i];
		std::string indexLabel = std::to_string(i);

		VkLabelObject(m_Device, VK_OBJECT_TYPE_COMMAND_BUFFER, buff, std::string("Graphics Renderer Command Buffer ") + indexLabel);
	}

	VkLabelObject(m_Device, VK_OBJECT_TYPE_INSTANCE, m_Instance, "Instance");
	VkLabelObject(m_Device, VK_OBJECT_TYPE_PHYSICAL_DEVICE, m_PhysicalDevice, "Physical Device");
	VkLabelObject(m_Device, VK_OBJECT_TYPE_DEVICE, m_Device, "Logical Device");	
	VkLabelObject(m_Device, VK_OBJECT_TYPE_SURFACE_KHR, m_Surface, "Window Surface");

	GPUMesh::CreateVertexInputDescriptions();

	CreateSamplers();
	CreateDefaultWhiteTexture();
	CreateDescriptorPool();
	Widget::CreateDescriptorSetLayout(m_Device, 512);
	Material::CreateDescriptorSetLayout(m_Device, 256);
	Animation::CreateDescriptorSetLayout(m_Device, 2048);
	Camera::CreateDescriptorSetLayout(m_Device, 32);

	CreateHDRResources();
	CreateHDRPipeline();

	CreateCompositingPipeline();

	CreateUIPipeline();

	CreateCSMRenderPass();
	CreateDefaultMaterial();

	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = m_Instance;
	initInfo.PhysicalDevice = m_PhysicalDevice;
	initInfo.Device = m_Device;
	initInfo.QueueFamily = m_GraphicsQueueIndex;
	initInfo.Queue = m_GraphicsQueue;
	initInfo.DescriptorPool = m_DescriptorPool;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator = nullptr;
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = m_SwapchainImages.size();
	initInfo.RenderPass = m_PostProcessRenderPass;
	initInfo.UseDynamicRendering = false;
	initInfo.CheckVkResultFn = [](VkResult result) { VkErrorCheck(result, "ImGui Vulkan Error!"); };
	CreateCompositingDescriptorResources();
	CreateBloomResources();
	RecreateBloomScreenResources();
	ImGui_ImplVulkan_Init(&initInfo);
	ImGui_ImplVulkan_CreateFontsTexture();
}

PBE::Renderer::~Renderer()
{
	vkDeviceWaitIdle(m_Device);
	ImGui_ImplVulkan_Shutdown();
	DestroySwapchainResources();
	DestroyBloomScreenResources();
	vkDestroyPipelineLayout(m_Device, m_BloomPipelineLayout, nullptr);
	vkDestroyPipeline(m_Device, m_BloomPipeline, nullptr);
	vkDestroyDescriptorSetLayout(m_Device, m_BloomDescriptorSetLayout, nullptr);
	vkDestroyRenderPass(m_Device,m_BloomRenderPass,nullptr);
	vkDestroyFence(m_Device, m_TransferFence, nullptr);

	vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);

	DestroyGPUBuffer(m_Allocator, m_StagingBuffer);
	delete m_DefaultMaterial;
	delete m_UIMaterial;
	FreeTexture(m_DefaultWhiteTexture);

	for (auto& gpuBufferList : m_InFlightStagingBuffers)
	{
		for (auto& buffer : gpuBufferList)
		{
			DestroyGPUBuffer(m_Allocator, buffer);
		}
		gpuBufferList.clear();
	}

	for (GPUBuffer& buffer : m_BuffersToDestroy)
	{
		DestroyGPUBuffer(m_Allocator, buffer);
	}
	m_BuffersToDestroy.clear();

	for (auto moduleLoadPair : m_LoadedShaderModules)
	{
		vkDestroyShaderModule( m_Device, moduleLoadPair.second, nullptr );
	}

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		FreeGPUMesh(m_ImmediateModeGPUMesh[i]);
		FreeGPUMesh(m_ImmediateModeUIGPUMesh[i]);
	}

	for(std::pair<TextureLoadInfo, Texture> texturePair : m_LoadedTextures)
	{
		vmaDestroyImage(m_Allocator, texturePair.second->m_Image, texturePair.second->m_Allocation);
		vkDestroyImageView(m_Device, texturePair.second->m_ImageView, nullptr);

		delete texturePair.second;
	}

	for (Texture texture : m_MemoryLoadedTextures)
	{
		vmaDestroyImage(m_Allocator, texture->m_Image, texture->m_Allocation);
		vkDestroyImageView(m_Device, texture->m_ImageView, nullptr);

		delete texture;
	}

	Widget::DestroyDescriptorSetLayout(m_Device);
	Material::DestroyDescriptorSetLayout(m_Device);
	Animation::DestroyDescriptorSetLayout(m_Device);
	Camera::DestroyDescriptorSetLayout(m_Device);

	DestroyHDRResources();

	vkDestroyDescriptorSetLayout(m_Device, m_CompositingDescriptorSetLayout, nullptr);
	vkDestroyPipeline(m_Device, m_CompositingPipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_CompositingPipelineLayout, nullptr);

	vkDestroyRenderPass(m_Device, m_PostProcessRenderPass, nullptr);

	vkDestroyPipeline(m_Device, m_StaticMeshPipeline, nullptr);
	vkDestroyPipeline(m_Device, m_SkinnedMeshPipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_HDRPipelineLayout, nullptr);

	vkDestroyPipeline(m_Device, m_UIPipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_UIPipelineLayout, nullptr);

	if (m_DebugMessenger != VK_NULL_HANDLE)
	{
		DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
	}

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(m_Device, m_ImageAvailableSemaphore[i], nullptr);
		vkDestroySemaphore(m_Device, m_RenderFinishedSemaphore[i], nullptr);
		vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
	}

	//Destroy all the samplers
	for (int i = 0; i < static_cast<int>(SamplerType::COUNT); ++i)
	{
		vkDestroySampler(m_Device, m_Samplers[i], nullptr);
	}

	vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);

	VmaShowDebugStats(m_Allocator);
	vmaDestroyAllocator(m_Allocator);
	vkDestroyDevice(m_Device, nullptr);
	vkDestroyInstance(m_Instance, nullptr);
}

PBE::Camera* PBE::Renderer::CreateCamera()
{
	Camera* camera = new Camera(this);
	return camera;
}

PBE::Material* PBE::Renderer::CreateMaterial(MaterialCreateInfo const& info)
{
	Material* material = new Material(info);
	
	return material;
}

std::shared_ptr<PBE::Material> PBE::Renderer::CreateMaterialShared(MaterialCreateInfo const& info)
{
	std::shared_ptr<Material> material = std::make_shared<Material>(info);
	return material;
}

template <typename T_ValueType>
void ResizeStagingBufferToFit(VkDevice device, VmaAllocator allocator, PBE::GPUBuffer& buffer, std::optional<std::vector<T_ValueType>> vertexStream)
{
	if (!vertexStream.has_value())
	{
		return;
	}

	//Grow staging buffer if needed
	if (buffer.m_Size < vertexStream.value().size() * sizeof(T_ValueType))
	{
		if (buffer.m_Buffer != VK_NULL_HANDLE)
		{
			PBE::DestroyGPUBuffer(allocator, buffer);
			buffer = {};
		}

		buffer = PBE::CreateGPUBuffer(
			allocator,
			vertexStream.value().size() * sizeof(T_ValueType),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);

		vmaMapMemory(allocator, buffer.m_Allocation, &buffer.m_Data);
		PBE::VkLabelObject(device, VK_OBJECT_TYPE_BUFFER, buffer.m_Buffer, "Staging Buffer");
		vmaSetAllocationName(allocator, buffer.m_Allocation, "Staging Buffer Allocation");
	}
}

PBE::GPUMesh* PBE::Renderer::CreateGPUMesh(CPUMesh const& cpuMesh, std::string_view debugName)
{
	GPUMesh* mesh = new GPUMesh();

	//Grow staging buffer if needed
	ResizeStagingBufferToFit(m_Device, m_Allocator, m_StagingBuffer, cpuMesh.m_Positions);
	ResizeStagingBufferToFit(m_Device, m_Allocator, m_StagingBuffer, cpuMesh.m_Normals);
	ResizeStagingBufferToFit(m_Device, m_Allocator, m_StagingBuffer, cpuMesh.m_Bitangents);
	ResizeStagingBufferToFit(m_Device, m_Allocator, m_StagingBuffer, cpuMesh.m_Tangents);
	ResizeStagingBufferToFit(m_Device, m_Allocator, m_StagingBuffer, cpuMesh.m_UVs);
	ResizeStagingBufferToFit(m_Device, m_Allocator, m_StagingBuffer, cpuMesh.m_Colors);
	ResizeStagingBufferToFit(m_Device, m_Allocator, m_StagingBuffer, cpuMesh.m_BoneWeights);
	ResizeStagingBufferToFit(m_Device, m_Allocator, m_StagingBuffer, cpuMesh.m_BoneIndices);

	if (cpuMesh.m_Positions.has_value() && cpuMesh.m_Positions->size() != 0)
	{
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::POSITION3D)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Positions.value().size() * sizeof(Vec3),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		memcpy(
			m_StagingBuffer.m_Data,
			cpuMesh.m_Positions.value().data(), 
			cpuMesh.m_Positions.value().size() * sizeof(Vec3)
		);

		SingleTimeCopyBuffer(
			m_Device,
			m_CommandPool, 
			m_GraphicsQueue, 
			m_StagingBuffer.m_Buffer, 
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::POSITION3D)]->m_Buffer, 
			cpuMesh.m_Positions.value().size() * sizeof(Vec3)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::POSITION3D)]->m_Buffer,
			std::string(debugName) + " Position Buffer"
		);
	}

	if (cpuMesh.m_Bitangents.has_value() && cpuMesh.m_Bitangents->size() != 0)
	{
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BITANGENT)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Bitangents.value().size() * sizeof(Vec3),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		memcpy(
			m_StagingBuffer.m_Data,
			cpuMesh.m_Bitangents.value().data(),
			cpuMesh.m_Bitangents.value().size() * sizeof(Vec3)
		);

		SingleTimeCopyBuffer(
			m_Device,
			m_CommandPool, 
			m_GraphicsQueue, 
			m_StagingBuffer.m_Buffer, 
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BITANGENT)]->m_Buffer, 
			cpuMesh.m_Bitangents.value().size() * sizeof(Vec3)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BITANGENT)]->m_Buffer,
			std::string(debugName) + " Bitangent Buffer"
		);
	}

	if (cpuMesh.m_Tangents.has_value() && cpuMesh.m_Tangents->size() != 0)
	{
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::TANGENT)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Tangents.value().size() * sizeof(Vec3),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		memcpy(
			m_StagingBuffer.m_Data,
			cpuMesh.m_Tangents.value().data(),
			cpuMesh.m_Tangents.value().size() * sizeof(Vec3)
		);

		SingleTimeCopyBuffer(
			m_Device,
			m_CommandPool, 
			m_GraphicsQueue, 
			m_StagingBuffer.m_Buffer, 
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::TANGENT)]->m_Buffer, 
			cpuMesh.m_Tangents.value().size() * sizeof(Vec3)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::TANGENT)]->m_Buffer,
			std::string(debugName) + " Tangent Buffer"
		);
	}

	if (cpuMesh.m_Normals.has_value() && cpuMesh.m_Normals->size() != 0)
	{
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::NORMAL)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Normals.value().size() * sizeof(Vec3),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		memcpy(
			m_StagingBuffer.m_Data,
			cpuMesh.m_Normals.value().data(),
			cpuMesh.m_Normals.value().size() * sizeof(Vec3)
		);

		SingleTimeCopyBuffer(
			m_Device, 
			m_CommandPool, 
			m_GraphicsQueue, 
			m_StagingBuffer.m_Buffer, 
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::NORMAL)]->m_Buffer, 
			cpuMesh.m_Normals.value().size() * sizeof(Vec3)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::NORMAL)]->m_Buffer,
			std::string(debugName) + " Normal Buffer"
		);
	}

	if (cpuMesh.m_UVs.has_value() && cpuMesh.m_UVs->size() != 0)
	{
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::UV)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_UVs.value().size() * sizeof(Vec2),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		memcpy(
			m_StagingBuffer.m_Data,
			cpuMesh.m_UVs.value().data(),
			cpuMesh.m_UVs.value().size() * sizeof(Vec2)
		);

		SingleTimeCopyBuffer(
			m_Device, 
			m_CommandPool, 
			m_GraphicsQueue, 
			m_StagingBuffer.m_Buffer, 
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::UV)]->m_Buffer, 
			cpuMesh.m_UVs.value().size() * sizeof(Vec2)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::UV)]->m_Buffer,
			std::string(debugName) + " UV Buffer"
		);
	}

	if (cpuMesh.m_Colors.has_value() && cpuMesh.m_Colors->size() != 0)
	{
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::COLOR)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Colors.value().size() * sizeof(Rgba8),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		memcpy(
			m_StagingBuffer.m_Data,
			cpuMesh.m_Colors.value().data(),
			cpuMesh.m_Colors.value().size() * sizeof(Rgba8)
		);

		SingleTimeCopyBuffer(
			m_Device,
			m_CommandPool, 
			m_GraphicsQueue, 
			m_StagingBuffer.m_Buffer, 
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::COLOR)]->m_Buffer, 
			cpuMesh.m_Colors.value().size() * sizeof(Rgba8)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::COLOR)]->m_Buffer,
			std::string(debugName) + " Color Buffer"
		);
	}

	if (cpuMesh.m_BoneWeights.has_value() && cpuMesh.m_BoneWeights->size() != 0)
	{
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_WEIGHTS)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_BoneWeights.value().size() * sizeof(BoneWeights),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		memcpy(
			m_StagingBuffer.m_Data,
			cpuMesh.m_BoneWeights.value().data(),
			cpuMesh.m_BoneWeights.value().size() * sizeof(BoneWeights)
		);

		SingleTimeCopyBuffer(
			m_Device,
			m_CommandPool, 
			m_GraphicsQueue, 
			m_StagingBuffer.m_Buffer, 
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_WEIGHTS)]->m_Buffer, 
			cpuMesh.m_BoneWeights.value().size() * sizeof(BoneWeights)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_WEIGHTS)]->m_Buffer,
			std::string(debugName) + " Bone Weight Buffer"
		);
	}

	if (cpuMesh.m_BoneIndices.has_value() && cpuMesh.m_BoneIndices->size() != 0)
	{
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_INDICES)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_BoneIndices.value().size() * sizeof(BoneIndices),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		memcpy(
			m_StagingBuffer.m_Data,
			cpuMesh.m_BoneIndices.value().data(),
			cpuMesh.m_BoneIndices.value().size() * sizeof(BoneIndices)
		);

		SingleTimeCopyBuffer(
			m_Device,
			m_CommandPool, 
			m_GraphicsQueue, 
			m_StagingBuffer.m_Buffer, 
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_INDICES)]->m_Buffer, 
			cpuMesh.m_BoneIndices.value().size() * sizeof(BoneIndices)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_INDICES)]->m_Buffer,
			std::string(debugName) + " Bone Index Buffer"
		);
	}

	if (m_StagingBuffer.m_Size < cpuMesh.m_Indices.size() * sizeof(uint32_t))
	{
		if (m_StagingBuffer.m_Buffer != VK_NULL_HANDLE)
		{
			DestroyGPUBuffer(m_Allocator, m_StagingBuffer);
			m_StagingBuffer = {};
		}

		m_StagingBuffer = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Indices.size() * sizeof(uint32_t),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_ONLY
		);

		vmaMapMemory(m_Allocator, m_StagingBuffer.m_Allocation, &m_StagingBuffer.m_Data);
	}

	mesh->m_IndexBuffer = CreateGPUBuffer(
		m_Allocator,
		cpuMesh.m_Indices.size() * sizeof(uint32_t),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY
	);

	memcpy(
		m_StagingBuffer.m_Data,
		cpuMesh.m_Indices.data(),
		cpuMesh.m_Indices.size() * sizeof(uint32_t)
	);

	SingleTimeCopyBuffer(
		m_Device,
		m_CommandPool,
		m_GraphicsQueue,
		m_StagingBuffer.m_Buffer,
		mesh->m_IndexBuffer.m_Buffer,
		cpuMesh.m_Indices.size() * sizeof(uint32_t)
	);

	VkLabelObject(
		m_Device,
		VK_OBJECT_TYPE_BUFFER,
		mesh->m_IndexBuffer.m_Buffer,
		std::string(debugName) + " Index Buffer"
	);

	mesh->m_IndexCount = static_cast<uint32_t>(cpuMesh.m_Indices.size());

	return mesh;
}

PBE::GPUBuffer PBE::Renderer::CmdCreateVertexBuffer(VkCommandBuffer cmd, uint32_t frameIndex, void const* data, size_t size, std::string_view debugName)
{
	GPUBuffer stage = CreateGPUBuffer(
		m_Allocator,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU
	);

	stage.Map(m_Allocator);

	memcpy(stage.m_Data, data, size);

	GPUBuffer buffer = CreateGPUBuffer(
		m_Allocator,
		size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY
	);

	PBE::AppendCopyBuffer(
		cmd,
		stage.m_Buffer,
		buffer.m_Buffer,
		size
	);

	VkLabelObject(
		m_Device,
		VK_OBJECT_TYPE_BUFFER,
		buffer.m_Buffer,
		std::string(debugName) + " Buffer"
	);

	m_InFlightStagingBuffers[frameIndex].push_back(stage);

	return buffer;
}

PBE::GPUMesh* PBE::Renderer::CmdCreateMesh(VkCommandBuffer cmd, uint32_t frameIndex, CPUMesh const& cpuMesh, std::string_view debugName)
{
	GPUMesh* mesh = new GPUMesh();

	if (cpuMesh.m_Positions.has_value() && cpuMesh.m_Positions->size() != 0)
	{
		GPUBuffer stage = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Positions.value().size() * sizeof(Vec3),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);
		stage.Map(m_Allocator);

		memcpy(
			stage.m_Data,
			cpuMesh.m_Positions.value().data(),
			cpuMesh.m_Positions.value().size() * sizeof(Vec3)
		);

		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::POSITION3D)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Positions.value().size() * sizeof(Vec3),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		PBE::AppendCopyBuffer(
			cmd, 
			stage.m_Buffer, 
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::POSITION3D)]->m_Buffer, 
			cpuMesh.m_Positions.value().size() * sizeof(Vec3)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::POSITION3D)]->m_Buffer,
			std::string(debugName) + " Position Buffer"
		);

		m_InFlightStagingBuffers[frameIndex].push_back(stage);
	}

	if (cpuMesh.m_Bitangents.has_value() && cpuMesh.m_Bitangents->size() != 0)
	{
		GPUBuffer stage = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Bitangents.value().size() * sizeof(Vec3),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);
		stage.Map(m_Allocator);

		memcpy(
			stage.m_Data,
			cpuMesh.m_Bitangents.value().data(),
			cpuMesh.m_Bitangents.value().size() * sizeof(Vec3)
		);

		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BITANGENT)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Bitangents.value().size() * sizeof(Vec3),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		PBE::AppendCopyBuffer(
			cmd,
			stage.m_Buffer,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BITANGENT)]->m_Buffer,
			cpuMesh.m_Bitangents.value().size() * sizeof(Vec3)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BITANGENT)]->m_Buffer,
			std::string(debugName) + " Bitangent Buffer"
		);

		m_InFlightStagingBuffers[frameIndex].push_back(stage);
	}

	if (cpuMesh.m_Tangents.has_value() && cpuMesh.m_Tangents->size() != 0)
	{
		GPUBuffer stage = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Tangents.value().size() * sizeof(Vec3),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);
		stage.Map(m_Allocator);

		memcpy(
			stage.m_Data,
			cpuMesh.m_Tangents.value().data(),
			cpuMesh.m_Tangents.value().size() * sizeof(Vec3)
		);

		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::TANGENT)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Tangents.value().size() * sizeof(Vec3),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		PBE::AppendCopyBuffer(
			cmd,
			stage.m_Buffer,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::TANGENT)]->m_Buffer,
			cpuMesh.m_Tangents.value().size() * sizeof(Vec3)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::TANGENT)]->m_Buffer,
			std::string(debugName) + " Tangent Buffer"
		);

		m_InFlightStagingBuffers[frameIndex].push_back(stage);
	}

	if (cpuMesh.m_Normals.has_value() && cpuMesh.m_Normals->size() != 0)
	{
		GPUBuffer stage = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Normals.value().size() * sizeof(Vec3),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);
		stage.Map(m_Allocator);

		memcpy(
			stage.m_Data,
			cpuMesh.m_Normals.value().data(),
			cpuMesh.m_Normals.value().size() * sizeof(Vec3)
		);

		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::NORMAL)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Normals.value().size() * sizeof(Vec3),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		PBE::AppendCopyBuffer(
			cmd,
			stage.m_Buffer,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::NORMAL)]->m_Buffer,
			cpuMesh.m_Normals.value().size() * sizeof(Vec3)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::NORMAL)]->m_Buffer,
			std::string(debugName) + " Normal Buffer"
		);

		m_InFlightStagingBuffers[frameIndex].push_back(stage);
	}

	if (cpuMesh.m_Colors.has_value() && cpuMesh.m_Colors->size() != 0)
	{
		GPUBuffer stage = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Colors.value().size() * sizeof(Rgba8),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);
		stage.Map(m_Allocator);

		memcpy(
			stage.m_Data,
			cpuMesh.m_Colors.value().data(),
			cpuMesh.m_Colors.value().size() * sizeof(Rgba8)
		);

		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::COLOR)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Colors.value().size() * sizeof(Rgba8),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		PBE::AppendCopyBuffer(
			cmd,
			stage.m_Buffer,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::COLOR)]->m_Buffer,
			cpuMesh.m_Colors.value().size() * sizeof(Rgba8)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::COLOR)]->m_Buffer,
			std::string(debugName) + " Color Buffer"
		);

		m_InFlightStagingBuffers[frameIndex].push_back(stage);
	
	}

	if (cpuMesh.m_UVs.has_value() && cpuMesh.m_UVs->size() != 0)
	{
		GPUBuffer stage = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_UVs.value().size() * sizeof(Vec2),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);
		stage.Map(m_Allocator);

		memcpy(
			stage.m_Data,
			cpuMesh.m_UVs.value().data(),
			cpuMesh.m_UVs.value().size() * sizeof(Vec2)
		);

		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::UV)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_UVs.value().size() * sizeof(Vec2),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		PBE::AppendCopyBuffer(
			cmd,
			stage.m_Buffer,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::UV)]->m_Buffer,
			cpuMesh.m_UVs.value().size() * sizeof(Vec2)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::UV)]->m_Buffer,
			std::string(debugName) + " UV Buffer"
		);

		m_InFlightStagingBuffers[frameIndex].push_back(stage);
	
	}

	if (cpuMesh.m_BoneIndices.has_value() && cpuMesh.m_BoneIndices->size() != 0)
	{
		GPUBuffer stage = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_BoneIndices.value().size() * sizeof(PBE::BoneIndices),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);
		stage.Map(m_Allocator);

		memcpy(
			stage.m_Data,
			cpuMesh.m_BoneIndices.value().data(),
			cpuMesh.m_BoneIndices.value().size() * sizeof(PBE::BoneIndices)
		);

		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_INDICES)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_BoneIndices.value().size() * sizeof(PBE::BoneIndices),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		PBE::AppendCopyBuffer(
			cmd,
			stage.m_Buffer,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_INDICES)]->m_Buffer,
			cpuMesh.m_BoneIndices.value().size() * sizeof(PBE::BoneIndices)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_INDICES)]->m_Buffer,
			std::string(debugName) + " Bone Index Buffer"
		);

		m_InFlightStagingBuffers[frameIndex].push_back(stage);
	}

	if (cpuMesh.m_BoneWeights.has_value() && cpuMesh.m_BoneWeights->size() != 0)
	{
		GPUBuffer stage = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_BoneWeights.value().size() * sizeof(PBE::BoneWeights),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);
		stage.Map(m_Allocator);

		memcpy(
			stage.m_Data,
			cpuMesh.m_BoneWeights.value().data(),
			cpuMesh.m_BoneWeights.value().size() * sizeof(PBE::BoneWeights)
		);

		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_WEIGHTS)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_BoneWeights.value().size() * sizeof(PBE::BoneWeights),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		PBE::AppendCopyBuffer(
			cmd,
			stage.m_Buffer,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_WEIGHTS)]->m_Buffer,
			cpuMesh.m_BoneWeights.value().size() * sizeof(PBE::BoneWeights)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_WEIGHTS)]->m_Buffer,
			std::string(debugName) + " Bone Weight Buffer"
		);

		m_InFlightStagingBuffers[frameIndex].push_back(stage);
	}

	if (cpuMesh.m_Indices.size() != 0)
	{
		GPUBuffer stage = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Indices.size() * sizeof(uint32_t),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);
		stage.Map(m_Allocator);

		memcpy(
			stage.m_Data,
			cpuMesh.m_Indices.data(),
			cpuMesh.m_Indices.size() * sizeof(uint32_t)
		);

		mesh->m_IndexBuffer = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Indices.size() * sizeof(uint32_t),
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		PBE::AppendCopyBuffer(
			cmd,
			stage.m_Buffer,
			mesh->m_IndexBuffer.m_Buffer,
			cpuMesh.m_Indices.size() * sizeof(uint32_t)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_IndexBuffer.m_Buffer,
			std::string(debugName) + " Index Buffer"
		);

		mesh->m_IndexCount = static_cast<uint32_t>(cpuMesh.m_Indices.size());

		m_InFlightStagingBuffers[frameIndex].push_back(stage);
	}

	return mesh;
}

void PBE::Renderer::AppendCopyBuffer(VkCommandBuffer cmd, uint32_t frameIndex, GPUBuffer buffer, void* pData, size_t size)
{
	GPUBuffer stage = CreateGPUBuffer(
		m_Allocator,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU
	);

	stage.Map(m_Allocator);

	memcpy(stage.m_Data, pData, size);

	PBE::AppendCopyBuffer(
		cmd,
		stage.m_Buffer,
		buffer.m_Buffer,
		size
	);

	m_InFlightStagingBuffers[frameIndex].push_back(stage);
}

PBE::GPUMesh* PBE::Renderer::CreateDynamicGPUMesh(CPUMesh const& cpuMesh, std::string_view debugName)
{
	GPUMesh* mesh = new GPUMesh();
	if (cpuMesh.m_Positions.has_value() && cpuMesh.m_Positions->size() != 0)
	{
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::POSITION3D)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Positions.value().size() * sizeof(Vec3),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::POSITION3D)]->Map(m_Allocator);
		memcpy(
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::POSITION3D)]->m_Data,
			cpuMesh.m_Positions.value().data(),
			cpuMesh.m_Positions.value().size() * sizeof(Vec3)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::POSITION3D)]->m_Buffer,
			std::string(debugName) + " Immediate Position Buffer"
		);
	}

	if (cpuMesh.m_Bitangents.has_value() && cpuMesh.m_Bitangents->size() != 0)
	{
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BITANGENT)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Bitangents.value().size() * sizeof(Vec3),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);

		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BITANGENT)]->Map(m_Allocator);
		memcpy(
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BITANGENT)]->m_Data,
			cpuMesh.m_Bitangents.value().data(),
			cpuMesh.m_Bitangents.value().size() * sizeof(Vec3)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BITANGENT)]->m_Buffer,
			std::string(debugName) + " Bitangent Buffer"
		);
	}

	if (cpuMesh.m_Tangents.has_value() && cpuMesh.m_Tangents->size() != 0)
	{
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::TANGENT)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Tangents.value().size() * sizeof(Vec3),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);

		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::TANGENT)]->Map(m_Allocator);
		memcpy(
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::TANGENT)]->m_Data,
			cpuMesh.m_Tangents.value().data(),
			cpuMesh.m_Tangents.value().size() * sizeof(Vec3)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::TANGENT)]->m_Buffer,
			std::string(debugName) + " Immediate Tangent Buffer"
		);
	}

	if (cpuMesh.m_Normals.has_value() && cpuMesh.m_Normals->size() != 0)
	{
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::NORMAL)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Normals.value().size() * sizeof(Vec3),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);

		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::NORMAL)]->Map(m_Allocator);

		memcpy(
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::NORMAL)]->m_Data,
			cpuMesh.m_Normals.value().data(),
			cpuMesh.m_Normals.value().size() * sizeof(Vec3)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::NORMAL)]->m_Buffer,
			std::string(debugName) + " Immediate Normal Buffer"
		);
	}

	if (cpuMesh.m_UVs.has_value() && cpuMesh.m_UVs->size() != 0)
	{
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::UV)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_UVs.value().size() * sizeof(Vec2),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);

		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::UV)]->Map(m_Allocator);
		memcpy(
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::UV)]->m_Data,
			cpuMesh.m_UVs.value().data(),
			cpuMesh.m_UVs.value().size() * sizeof(Vec2)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::UV)]->m_Buffer,
			std::string(debugName) + " Immediate UV Buffer"
		);
	}

	if (cpuMesh.m_Colors.has_value() && cpuMesh.m_Colors->size() != 0)
	{
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::COLOR)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_Colors.value().size() * sizeof(Rgba8),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);

		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::COLOR)]->Map(m_Allocator);

		memcpy(
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::COLOR)]->m_Data,
			cpuMesh.m_Colors.value().data(),
			cpuMesh.m_Colors.value().size() * sizeof(Rgba8)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::COLOR)]->m_Buffer,
			std::string(debugName) + " Immediate Color Buffer"
		);
	}

	if (cpuMesh.m_BoneWeights.has_value() && cpuMesh.m_BoneWeights->size() != 0)
	{
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_WEIGHTS)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_BoneWeights.value().size() * sizeof(BoneWeights),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);

		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_WEIGHTS)]->Map(m_Allocator);

		memcpy(
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_WEIGHTS)]->m_Data,
			cpuMesh.m_BoneWeights.value().data(),
			cpuMesh.m_BoneWeights.value().size() * sizeof(BoneWeights)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_WEIGHTS)]->m_Buffer,
			std::string(debugName) + " Bone Weight Buffer"
		);
	}

	if (cpuMesh.m_BoneIndices.has_value() && cpuMesh.m_BoneIndices->size() != 0)
	{
		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_INDICES)] = CreateGPUBuffer(
			m_Allocator,
			cpuMesh.m_BoneIndices.value().size() * sizeof(BoneIndices),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU
		);

		mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_INDICES)]->Map(m_Allocator);

		memcpy(
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_INDICES)]->m_Data,
			cpuMesh.m_BoneIndices.value().data(),
			cpuMesh.m_BoneIndices.value().size() * sizeof(BoneIndices)
		);

		VkLabelObject(
			m_Device,
			VK_OBJECT_TYPE_BUFFER,
			mesh->m_VertexBuffers[static_cast<size_t>(VertexAttribute::BONE_INDICES)]->m_Buffer,
			std::string(debugName) + " Immediate Bone Index Buffer"
		);
	}

	mesh->m_IndexBuffer = CreateGPUBuffer(
		m_Allocator,
		cpuMesh.m_Indices.size() * sizeof(uint32_t),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU
	);

	mesh->m_IndexBuffer.Map(m_Allocator);

	memcpy(
		mesh->m_IndexBuffer.m_Data,
		cpuMesh.m_Indices.data(),
		cpuMesh.m_Indices.size() * sizeof(uint32_t)
	);

	VkLabelObject(
		m_Device,
		VK_OBJECT_TYPE_BUFFER,
		mesh->m_IndexBuffer.m_Buffer,
		std::string(debugName) + " Immediate Index Buffer"
	);

	mesh->m_IndexCount = static_cast<uint32_t>(cpuMesh.m_Indices.size());

	return mesh;
}

void PBE::Renderer::FreeGPUMesh(GPUMesh* mesh)
{
	if (mesh == nullptr)
	{
		return;
	}

	for (int i = 0; i < static_cast<int>(VertexAttribute::COUNT); ++i)
	{
		if (mesh->m_VertexBuffers[i].has_value())
		{
			m_BuffersToDestroy.push_back(mesh->m_VertexBuffers[i].value());
		}
	}

	m_BuffersToDestroy.push_back(mesh->m_IndexBuffer);

	delete mesh;

}

void PBE::Renderer::WaitIdle() const
{
	vkDeviceWaitIdle(m_Device);
}

VkShaderModule PBE::Renderer::CreateOrGetShaderModule(ShaderModuleLoadInfo const& info)
{
	auto it = m_LoadedShaderModules.find(info);
	if (it != m_LoadedShaderModules.end())
	{
		return it->second;
	}

	glslang_stage_t stage;
	switch (info.m_Stage)
	{
		case ShaderStage::VERTEX: stage = GLSLANG_STAGE_VERTEX; break;
		case ShaderStage::FRAGMENT: stage = GLSLANG_STAGE_FRAGMENT; break;
		case ShaderStage::COMPUTE: stage = GLSLANG_STAGE_COMPUTE; break;
		case ShaderStage::GEOMETRY: stage = GLSLANG_STAGE_GEOMETRY; break;
		default: throw std::logic_error("Stage not implemented"); break;
	}

	VkShaderModule shaderModule = PBE::CreateOrGetShaderModule(m_Device,stage,info.m_Path);
	m_LoadedShaderModules[info] = shaderModule;

	return shaderModule;
}

PBE::Texture2D PBE::Renderer::LoadOrGetTexture2D(std::filesystem::path path, SamplerType type)
{
	PBE::TextureLoadInfo info
	{
		.m_Path = path,
		.m_SamplerType = type,
		.m_TextureType = TextureType::Texture2D
	};

	auto it = m_LoadedTextures.find(info);
	if (it != m_LoadedTextures.end())
	{
		return (Texture2D)(it->second);
	}

	Texture2D texture = LoadTexture2D(path, type);
	if (texture == m_DefaultWhiteTexture)
	{
		std::cout << "Could not load texture at " << path << "!" << '\n';
		return texture;
	}
	m_LoadedTextures[info] = texture;

	return texture;
}

PBE::Texture2D PBE::Renderer::LoadTexture2D(std::filesystem::path path, SamplerType type)
{
	int width, height, channels;

	stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
	
	if (pixels == nullptr)
	{
		return (Texture2D)m_DefaultWhiteTexture;
	}

	Texture2D texture = CreateTexture2D(
		path.filename().string(),
		VkExtent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) },
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_SAMPLE_COUNT_1_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY,
		type
	);

	VkDeviceSize imageSize = width * height * 4;

	if (m_StagingBuffer.m_Size < imageSize)
	{
		if (m_StagingBuffer.m_Buffer != VK_NULL_HANDLE)
		{
			DestroyGPUBuffer(m_Allocator, m_StagingBuffer);
			m_StagingBuffer = {};
		}

		m_StagingBuffer = CreateGPUBuffer(
			m_Allocator,
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_ONLY
		);

		VkLabelObject(m_Device, VK_OBJECT_TYPE_BUFFER, m_StagingBuffer.m_Buffer, "Staging Buffer");

		vmaMapMemory(m_Allocator, m_StagingBuffer.m_Allocation, &m_StagingBuffer.m_Data);
	}

	memcpy(m_StagingBuffer.m_Data, pixels, imageSize);

	stbi_image_free(pixels);

	TransitionImageLayout(
		m_Device,
		m_CommandPool,
		m_GraphicsQueue,
		texture->m_Image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	CopyBufferToImage(
		m_Device,
		m_CommandPool,
		m_GraphicsQueue,
		m_StagingBuffer.m_Buffer,
		m_StagingBuffer.m_Size,
		texture->m_Image,
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	);

	GenerateMipmaps(
		m_Device,
		m_CommandPool,
		m_GraphicsQueue,
		texture->m_Image,
		width,
		height,
		texture->m_MipLevels
	);

	TransitionImageLayout(
		m_Device,
		m_CommandPool,
		m_GraphicsQueue,
		texture->m_Image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	VkLabelObject(m_Device, VK_OBJECT_TYPE_IMAGE, texture->m_Image, path.filename().string());

	return texture;
}

PBE::Texture2D PBE::Renderer::LoadFromMemoryTexture2D(
	std::string_view name,
	VkExtent2D extent,
	void const* data, 
	SamplerType type
)
{
	Texture2D texture = CreateTexture2D(
		std::string(name),
		extent,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_SAMPLE_COUNT_1_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY,
		type
	);

	VkDeviceSize imageSize = extent.width * extent.height * 4;

	if (m_StagingBuffer.m_Size < imageSize)
	{
		if (m_StagingBuffer.m_Buffer != VK_NULL_HANDLE)
		{
			DestroyGPUBuffer(m_Allocator, m_StagingBuffer);
			m_StagingBuffer = {};
		}

		m_StagingBuffer = CreateGPUBuffer(
			m_Allocator,
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_ONLY
		);

		VkLabelObject(m_Device, VK_OBJECT_TYPE_BUFFER, m_StagingBuffer.m_Buffer, "Staging Buffer");

		vmaMapMemory(m_Allocator, m_StagingBuffer.m_Allocation, &m_StagingBuffer.m_Data);
	}

	memcpy(m_StagingBuffer.m_Data, data, imageSize);

	TransitionImageLayout(
		m_Device,
		m_CommandPool,
		m_GraphicsQueue,
		texture->m_Image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	CopyBufferToImage(
		m_Device,
		m_CommandPool,
		m_GraphicsQueue,
		m_StagingBuffer.m_Buffer,
		m_StagingBuffer.m_Size,
		texture->m_Image,
		static_cast<uint32_t>(extent.width),
		static_cast<uint32_t>(extent.height)
	);

	GenerateMipmaps(
		m_Device,
		m_CommandPool,
		m_GraphicsQueue,
		texture->m_Image,
		extent.width,
		extent.height,
		texture->m_MipLevels
	);

	TransitionImageLayout(
		m_Device,
		m_CommandPool,
		m_GraphicsQueue,
		texture->m_Image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	VkLabelObject(m_Device, VK_OBJECT_TYPE_IMAGE, texture->m_Image, name);

	texture->m_Sampler = m_Samplers[(size_t)type];
	m_MemoryLoadedTextures.push_back(texture);

	return texture;
}

PBE::Texture2D_T* PBE::Renderer::CreateTexture2D(
	std::string_view name,
	VkExtent2D extent, 
	VkFormat format, 
	VkImageTiling tiling, 
	VkImageUsageFlags usage, 
	VkSampleCountFlagBits samples,
	VmaMemoryUsage memoryUsage,
	SamplerType type,
	VkImageAspectFlagBits aspectMask
)
{
	Texture2D_T* texture = new Texture2D_T();
	
	VkImageCreateInfo imageInfo
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.extent = { extent.width, extent.height, 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
	};
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = samples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo
	{
		.usage = memoryUsage,
	};

	VkErrorCheck(vmaCreateImage(m_Allocator, &imageInfo, &allocInfo, &texture->m_Image, &texture->m_Allocation, nullptr));
	vmaSetAllocationName(m_Allocator, texture->m_Allocation, name.data());
	texture->m_Height = extent.height;
	texture->m_Width = extent.width;
	texture->m_MipLevels = 1;
	texture->m_LayerCount = 1;

	VkImageViewCreateInfo imageView = {};
	imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageView.image = texture->m_Image;
	imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageView.format = format;
	imageView.subresourceRange.aspectMask = aspectMask;
	imageView.subresourceRange.baseMipLevel = 0;
	imageView.subresourceRange.levelCount = 1;
	imageView.subresourceRange.baseArrayLayer = 0;
	imageView.subresourceRange.layerCount = 1;

	VkErrorCheck(vkCreateImageView(m_Device, &imageView, nullptr, &texture->m_ImageView));

	VkLabelObject(m_Device, VK_OBJECT_TYPE_IMAGE, texture->m_Image, name);
	std::string texViewNameStr(std::string(name) + " View");
	VkLabelObject(m_Device, VK_OBJECT_TYPE_IMAGE_VIEW, texture->m_ImageView, texViewNameStr);
	std::string texAllocationNameStr(std::string(name) + " Allocation");
	vmaSetAllocationName(m_Allocator, texture->m_Allocation, texAllocationNameStr.c_str());

	texture->m_Sampler = m_Samplers[(size_t)type];

	return texture;
}

PBE::Texture2DArray PBE::Renderer::CreateTexture2DArray(
	std::string_view name, 
	VkExtent3D extent, 
	uint32_t layerCount, 
	VkFormat format, 
	VkImageTiling tiling, 
	VkImageUsageFlags usage, 
	VkSampleCountFlagBits samples, 
	VmaMemoryUsage memoryUsage, 
	SamplerType type,
	VkImageAspectFlagBits aspectMask
)
{
	Texture2DArray texture = new Texture2DArray_T();

	VkImageCreateInfo imageInfo
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.extent = extent,
		.mipLevels = 1,
		.arrayLayers = layerCount,
	};

	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = samples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo
	{
		.usage = memoryUsage,
	};
	
	VkErrorCheck(vmaCreateImage(m_Allocator, &imageInfo, &allocInfo, &texture->m_Image, &texture->m_Allocation, nullptr));
	vmaSetAllocationName(m_Allocator, texture->m_Allocation, name.data());


	texture->m_Height = extent.height;
	texture->m_Width = extent.width;
	texture->m_LayerCount = layerCount;
	texture->m_MipLevels = 1;

	VkImageViewCreateInfo imageView = {};
	imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageView.image = texture->m_Image;
	imageView.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	imageView.format = format;
	imageView.subresourceRange.aspectMask = aspectMask;
	imageView.subresourceRange.baseMipLevel = 0;
	imageView.subresourceRange.levelCount = 1;
	imageView.subresourceRange.baseArrayLayer = 0;
	imageView.subresourceRange.layerCount = layerCount;

	VkErrorCheck(vkCreateImageView(m_Device, &imageView, nullptr, &texture->m_ImageView));

	texture->m_Sampler = m_Samplers[(size_t)type];

	return texture;
}

void PBE::Renderer::FreeTexture(Texture texture)
{
	//Remove from lists use to manage automatic cleanup
	for (auto itr = m_MemoryLoadedTextures.begin(); itr != m_MemoryLoadedTextures.end(); ++itr)
	{
		Texture tex = *itr;
		if (tex == texture)
		{
			m_MemoryLoadedTextures.erase(itr);
			break;
		}
	}

	for (auto itr = m_LoadedTextures.begin(); itr != m_LoadedTextures.end(); ++itr)
	{
		if (itr->second == texture)
		{
			m_LoadedTextures.erase(itr);
			break;
		}
	}

	vmaDestroyImage(m_Allocator, texture->m_Image, texture->m_Allocation);
	vkDestroyImageView(m_Device, texture->m_ImageView, nullptr);

	delete texture;
}

void PBE::Renderer::BeginFrame()
{
}

void PBE::Renderer::SubmitCmdDraw()
{
	ImGui::Render();
	m_PreRenderEvent.Invoke();
	uint32_t imageIndex;
	vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

	VkResult result = vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphore[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		auto window = m_Info.m_pWindow->GetWindow();
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}
		RecreateScreenResources();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		VkErrorCheck(result);
	}

	vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);
	RecordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex, m_CurrentFrame);

	VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore[m_CurrentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphore[m_CurrentFrame] };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VkErrorCheck(vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]));

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapchains[] = { m_Swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;

	presentInfo.pImageIndices = &imageIndex;

	VkResult result2 = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
	
	if (result2 == VK_ERROR_OUT_OF_DATE_KHR || result2 == VK_SUBOPTIMAL_KHR)
	{
		auto window = m_Info.m_pWindow->GetWindow();
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}
		RecreateScreenResources();
	}
	else if (result2 != VK_SUCCESS)
	{
		VkErrorCheck(result);
	}

	m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void PBE::Renderer::CreateHDRResources()
{
	VkImageCreateInfo hdrImageInfo{};
	hdrImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	hdrImageInfo.imageType = VK_IMAGE_TYPE_2D;
	hdrImageInfo.extent.width = m_SwapchainExtent.width;
	hdrImageInfo.extent.height = m_SwapchainExtent.height;
	hdrImageInfo.extent.depth = 1;
	hdrImageInfo.mipLevels = 1;
	hdrImageInfo.arrayLayers = 1;
	hdrImageInfo.format = m_HDRFormat;
	hdrImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	hdrImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	hdrImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	hdrImageInfo.samples = m_MsaaSamples;

	hdrImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	hdrImageInfo.flags = 0;

	VmaAllocationCreateInfo hdrImageAllocInfo{};
	hdrImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkErrorCheck(vmaCreateImage(m_Allocator, &hdrImageInfo, &hdrImageAllocInfo, &m_HDRImage, &m_HDRImageAllocation, nullptr));
	vmaSetAllocationName(m_Allocator, m_HDRImageAllocation, "HDR Image Allocation");

	VkImageViewCreateInfo hdrImageViewInfo{};
	hdrImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	hdrImageViewInfo.image = m_HDRImage;
	hdrImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	hdrImageViewInfo.format = m_HDRFormat;

	hdrImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	hdrImageViewInfo.subresourceRange.baseMipLevel = 0;
	hdrImageViewInfo.subresourceRange.levelCount = 1;
	hdrImageViewInfo.subresourceRange.baseArrayLayer = 0;
	hdrImageViewInfo.subresourceRange.layerCount = 1;

	VkErrorCheck(vkCreateImageView(m_Device, &hdrImageViewInfo, nullptr, &m_HDRImageView));

	VkImageViewCreateInfo hdrStencilImageViewInfo{};
	hdrStencilImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	hdrStencilImageViewInfo.image = m_HDRImage;
	hdrStencilImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	hdrStencilImageViewInfo.format = VK_FORMAT_D16_UNORM_S8_UINT;

	hdrStencilImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
	hdrStencilImageViewInfo.subresourceRange.levelCount = 1;
	hdrStencilImageViewInfo.subresourceRange.baseArrayLayer = 0;
	hdrStencilImageViewInfo.subresourceRange.layerCount = 1;

	VkErrorCheck(vkCreateImageView(m_Device, &hdrStencilImageViewInfo, nullptr, &m_HDRStencilImageView));

	VkLabelObject(m_Device, VK_OBJECT_TYPE_IMAGE, m_HDRImage, "HDR Image");
	VkLabelObject(m_Device, VK_OBJECT_TYPE_IMAGE_VIEW, m_HDRImageView, "HDR Image View");

	VkImageCreateInfo depthImageInfo{};
	depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
	depthImageInfo.extent.width = m_SwapchainExtent.width;
	depthImageInfo.extent.height = m_SwapchainExtent.height;
	depthImageInfo.extent.depth = 1;
	depthImageInfo.mipLevels = 1;
	depthImageInfo.arrayLayers = 1;
	depthImageInfo.format = FindDepthFormat(m_PhysicalDevice);
	depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	depthImageInfo.samples = m_MsaaSamples;

	depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	depthImageInfo.flags = 0;

	VmaAllocationCreateInfo depthImageAllocInfo{};
	depthImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkErrorCheck(vmaCreateImage(m_Allocator, &depthImageInfo, &depthImageAllocInfo, &m_DepthImage, &m_DepthImageAllocation, nullptr));
	vmaSetAllocationName(m_Allocator, m_DepthImageAllocation, "Depth Image Allocation");

	VkImageViewCreateInfo depthImageViewInfo{};
	depthImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthImageViewInfo.image = m_DepthImage;
	depthImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthImageViewInfo.format = FindDepthFormat(m_PhysicalDevice);

	depthImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthImageViewInfo.subresourceRange.baseMipLevel = 0;
	depthImageViewInfo.subresourceRange.levelCount = 1;
	depthImageViewInfo.subresourceRange.baseArrayLayer = 0;
	depthImageViewInfo.subresourceRange.layerCount = 1;

	VkErrorCheck(vkCreateImageView(m_Device, &depthImageViewInfo, nullptr, &m_DepthImageView));

	VkLabelObject(m_Device, VK_OBJECT_TYPE_IMAGE, m_DepthImage, "Depth Image");
	VkLabelObject(m_Device, VK_OBJECT_TYPE_IMAGE_VIEW, m_DepthImageView, "Depth Image View");

	VkImageCreateInfo hdrResolveImageInfo{};
	hdrResolveImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	hdrResolveImageInfo.imageType = VK_IMAGE_TYPE_2D;
	hdrResolveImageInfo.extent.width = m_SwapchainExtent.width;
	hdrResolveImageInfo.extent.height = m_SwapchainExtent.height;
	hdrResolveImageInfo.extent.depth = 1;
	hdrResolveImageInfo.mipLevels = 1;
	hdrResolveImageInfo.arrayLayers = 1;
	hdrResolveImageInfo.format = m_HDRResolveFormat;
	hdrResolveImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	hdrResolveImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	hdrResolveImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	hdrResolveImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	hdrResolveImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo hdrResolveImageAllocInfo{};
	hdrResolveImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkErrorCheck(vmaCreateImage(m_Allocator, &hdrResolveImageInfo, &hdrResolveImageAllocInfo, &m_HDRResolveImage, &m_HDRResolveImageAllocation, nullptr));
	vmaSetAllocationName(m_Allocator, m_HDRResolveImageAllocation, "HDR Resolve Image Allocation");

	VkImageViewCreateInfo hdrResolveImageViewInfo{};

	hdrResolveImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	hdrResolveImageViewInfo.image = m_HDRResolveImage;
	hdrResolveImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	hdrResolveImageViewInfo.format = m_HDRResolveFormat;

	hdrResolveImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	hdrResolveImageViewInfo.subresourceRange.baseMipLevel = 0;
	hdrResolveImageViewInfo.subresourceRange.levelCount = 1;
	hdrResolveImageViewInfo.subresourceRange.baseArrayLayer = 0;
	hdrResolveImageViewInfo.subresourceRange.layerCount = 1;

	VkErrorCheck(vkCreateImageView(m_Device, &hdrResolveImageViewInfo, nullptr, &m_HDRResolveImageView));

	VkLabelObject(m_Device, VK_OBJECT_TYPE_IMAGE, m_HDRResolveImage, "HDR Resolve Image");
	VkLabelObject(m_Device, VK_OBJECT_TYPE_IMAGE_VIEW, m_HDRResolveImageView, "HDR Resolve Image View");
	vmaSetAllocationName(m_Allocator , m_HDRResolveImageAllocation, "HDR Resolve Image Allocation");

	m_HDRResolveDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	m_HDRResolveDescriptor.imageView = m_HDRResolveImageView;
	m_HDRResolveDescriptor.sampler = m_Samplers[static_cast<size_t>(SamplerType::LINEAR_CLAMP)];
	m_HDRResolveDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	CreateHDRRenderPass();
	CreateHDRFramebuffer();
}

void PBE::Renderer::CreateBloomRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_HDRFormat; // must match the format of your bloom images
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VkErrorCheck(vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_BloomRenderPass));
	VkLabelObject(m_Device, VK_OBJECT_TYPE_RENDER_PASS, m_BloomRenderPass, "Bloom Render Pass");
}

void PBE::Renderer::CreateBloomResources()
{
	uint32_t numMipLevels = m_BloomPassCount;
	CreateBloomRenderPass();
	CreateBloomPipeline();

	m_BloomDescriptorSets.resize(m_BloomPassCount);

	std::vector<VkDescriptorSetLayout> layouts(m_BloomPassCount, m_BloomDescriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	allocInfo.pSetLayouts = layouts.data();

	VkErrorCheck(vkAllocateDescriptorSets(m_Device, &allocInfo, m_BloomDescriptorSets.data()));
}

void PBE::Renderer::CreateBloomScreenResources()
{
	uint32_t numMipLevels = m_BloomPassCount;
	m_BloomImages.resize(numMipLevels);
	m_BloomImageViews.resize(numMipLevels);
	m_BloomFramebuffers.resize(numMipLevels);
	m_BloomAllocations.resize(numMipLevels);
	m_BloomExtents.resize(numMipLevels);

	for (uint32_t mip = 0; mip < numMipLevels; ++mip)
	{
		VkExtent2D mipExtent = {
			std::max(1u, m_SwapchainExtent.width >> (mip + 1)),  // Half-size base
			std::max(1u, m_SwapchainExtent.height >> (mip + 1))
		};

		m_BloomExtents[mip] = mipExtent;

		VkImage& bloomImage = m_BloomImages[mip];
		VkImageView& bloomImageView = m_BloomImageViews[mip];
		VmaAllocation& bloomImageAllocation = m_BloomAllocations[mip];

		VkImageCreateInfo bloomImageInfo{};
		bloomImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		bloomImageInfo.imageType = VK_IMAGE_TYPE_2D;
		bloomImageInfo.extent.width = mipExtent.width;
		bloomImageInfo.extent.height = mipExtent.height;
		bloomImageInfo.extent.depth = 1;
		bloomImageInfo.mipLevels = 1;
		bloomImageInfo.arrayLayers = 1;
		bloomImageInfo.format = m_HDRFormat;
		bloomImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		bloomImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		bloomImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		bloomImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		bloomImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bloomImageInfo.flags = 0;

		VmaAllocationCreateInfo bloomImageAllocInfo{};
		bloomImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		VkErrorCheck(vmaCreateImage(m_Allocator, &bloomImageInfo, &bloomImageAllocInfo, &bloomImage, &bloomImageAllocation, nullptr));
		vmaSetAllocationName(m_Allocator, bloomImageAllocation, "Bloom Image Allocation");

		VkImageViewCreateInfo bloomImageViewInfo{};
		bloomImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		bloomImageViewInfo.image = bloomImage;
		bloomImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		bloomImageViewInfo.format = m_HDRFormat;
		bloomImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bloomImageViewInfo.subresourceRange.baseMipLevel = 0;
		bloomImageViewInfo.subresourceRange.levelCount = 1;
		bloomImageViewInfo.subresourceRange.baseArrayLayer = 0;
		bloomImageViewInfo.subresourceRange.layerCount = 1;

		VkErrorCheck(vkCreateImageView(m_Device, &bloomImageViewInfo, nullptr, &bloomImageView));

		VkLabelObject(m_Device, VK_OBJECT_TYPE_IMAGE, bloomImage, "Bloom Image");
		VkLabelObject(m_Device, VK_OBJECT_TYPE_IMAGE_VIEW, bloomImageView, "Bloom Image View");

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_BloomRenderPass; // Presumed to be initialized elsewhere
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &bloomImageView;
		framebufferInfo.width = mipExtent.width;
		framebufferInfo.height = mipExtent.height;
		framebufferInfo.layers = 1;

		VkErrorCheck(vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_BloomFramebuffers[mip]));

		VkLabelObject(m_Device, VK_OBJECT_TYPE_FRAMEBUFFER, m_BloomFramebuffers[mip], "Bloom Framebuffer");
	}
	for (size_t i = 0; i < m_BloomImageViews.size(); ++i)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		if (i == 0)
		{
			imageInfo.imageView = m_HDRResolveImageView;
		}
		else
		{
			imageInfo.imageView = m_BloomImageViews[i - 1];
		}
		imageInfo.sampler = m_Samplers[static_cast<size_t>(SamplerType::LINEAR_CLAMP)];

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = m_BloomDescriptorSets[i];
		write.dstBinding = 0;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.descriptorCount = 1;
		write.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_Device, 1, &write, 0, nullptr);
	}

}

void PBE::Renderer::DestroyBloomScreenResources()
{
	for (size_t i = 0; i < m_BloomImageViews.size(); ++i)
	{
		vkDestroyFramebuffer(m_Device,m_BloomFramebuffers[i],nullptr);
		m_BloomFramebuffers[i] = VK_NULL_HANDLE;
		vmaDestroyImage(m_Allocator, m_BloomImages[i], m_BloomAllocations[i]);
		m_BloomImages[i] = VK_NULL_HANDLE;
		vkDestroyImageView(m_Device, m_BloomImageViews[i], nullptr);
		m_BloomImageViews[i] = VK_NULL_HANDLE;
	}
}

void PBE::Renderer::RecreateBloomScreenResources()
{
	DestroyBloomScreenResources();
	CreateBloomScreenResources();
	// --- Binding 1: Array of Bloom Mip ImageInfos ---
	std::vector<VkDescriptorImageInfo> bloomImageInfos;
	bloomImageInfos.reserve(m_BloomImageViews.size());

	for (VkImageView view : m_BloomImageViews)
	{
		VkDescriptorImageInfo info{};
		info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		info.imageView = view;
		info.sampler = m_Samplers[static_cast<size_t>(SamplerType::LINEAR_CLAMP)];
		bloomImageInfos.push_back(info);
	}

	VkWriteDescriptorSet compositeWrite = {};
	compositeWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	compositeWrite.dstSet = m_CompositingDescriptorSet;
	compositeWrite.dstBinding = 1;
	compositeWrite.dstArrayElement = 0;
	compositeWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	compositeWrite.descriptorCount = static_cast<uint32_t>(bloomImageInfos.size());
	compositeWrite.pImageInfo = bloomImageInfos.data();

	vkUpdateDescriptorSets(m_Device, 1, &compositeWrite, 0, VK_NULL_HANDLE);
}

void PBE::Renderer::CreateHDRRenderPass()
{
	VkAttachmentDescription hdrAttachment
	{
		.format = m_HDRFormat,
		.samples = m_MsaaSamples,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentReference hdrAttachmentRef
	{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentDescription depthAttachment
	{
		.format = FindDepthFormat(m_PhysicalDevice),
		.samples = m_MsaaSamples,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentReference depthAttachmentRef
	{
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentDescription colorAttachment
	{
		.format = m_HDRResolveFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};

	VkAttachmentReference colorAttachmentRef
	{
		.attachment = 2,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	std::array<VkAttachmentDescription, 3> attachments = { hdrAttachment, depthAttachment, colorAttachment };

	VkSubpassDescription hdrSubpass
	{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &hdrAttachmentRef,
		.pDepthStencilAttachment = &depthAttachmentRef,
	};
	hdrSubpass.pResolveAttachments = &colorAttachmentRef;

	VkSubpassDependency hdrDependency{};
	hdrDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	hdrDependency.dstSubpass = 0;
	hdrDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	hdrDependency.srcAccessMask = 0;
	hdrDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	hdrDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkSubpassDescription, 1> subpasses = { hdrSubpass };

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = (uint32_t)attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = (uint32_t)subpasses.size();
	renderPassInfo.pSubpasses = subpasses.data();
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &hdrDependency;

	VkErrorCheck(vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_HDRRenderPass));

	VkLabelObject(m_Device, VK_OBJECT_TYPE_RENDER_PASS, m_HDRRenderPass, "HDR RenderPass");
}

void PBE::Renderer::CreateHDRFramebuffer()
{
	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = m_HDRRenderPass;
	framebufferInfo.attachmentCount = 2;
	framebufferInfo.width = m_SwapchainExtent.width;
	framebufferInfo.height = m_SwapchainExtent.height;

	framebufferInfo.layers = 1;

	std::array<VkImageView, 3> attachments = { m_HDRImageView, m_DepthImageView, m_HDRResolveImageView };

	framebufferInfo.attachmentCount = (uint32_t)attachments.size();
	framebufferInfo.pAttachments = attachments.data();

	VkErrorCheck(vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_HDRFramebuffer));
	VkLabelObject(m_Device, VK_OBJECT_TYPE_FRAMEBUFFER, m_HDRFramebuffer, "HDR Framebuffer");
}

void PBE::Renderer::CreateHDRPipeline()
{
	std::array<VkDescriptorSetLayout, 3> layouts = { Material::s_DescriptorSetLayout, Animation::s_DescriptorSetLayout, Camera::s_DescriptorSetLayout };
	std::array<VkPushConstantRange, 1> pushConstants = {  };

	pushConstants[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
	pushConstants[0].offset = 0;
	pushConstants[0].size = sizeof(CameraModelPushConstantData);

	VkPipelineLayoutCreateInfo hdrLayoutInfo{};
	hdrLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	hdrLayoutInfo.setLayoutCount = (uint32_t)layouts.size();
	hdrLayoutInfo.pSetLayouts = layouts.data();
	hdrLayoutInfo.pushConstantRangeCount = (uint32_t)pushConstants.size();
	hdrLayoutInfo.pPushConstantRanges = pushConstants.data();

	VkErrorCheck(vkCreatePipelineLayout(m_Device, &hdrLayoutInfo, nullptr, &m_HDRPipelineLayout));

	VkShaderModule vertex, fragment;
	ShaderModuleLoadInfo info;

	info.m_Path = "Assets/Shaders/SkinnedMesh.vert";
	info.m_Stage = ShaderStage::VERTEX;

	vertex = CreateOrGetShaderModule(info);

	info.m_Path = "Assets/Shaders/SkinnedMesh.frag";
	info.m_Stage = ShaderStage::FRAGMENT;

	fragment = CreateOrGetShaderModule(info);

	std::vector<VkVertexInputBindingDescription> bindingDescriptions =
	{
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::POSITION3D],
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::NORMAL],
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::UV],
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::COLOR],
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::TANGENT],
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::BITANGENT],
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::BONE_WEIGHTS],
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::BONE_INDICES],
	};

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions =
	{
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::POSITION3D],
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::NORMAL],
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::UV],
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::COLOR],
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::TANGENT],
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::BITANGENT],
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::BONE_WEIGHTS],
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::BONE_INDICES],
	};

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);

	shaderStages[0].module = vertex;
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].pName = "main";

	shaderStages[1].module = fragment;
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].pName = "main";

	GraphicsPipelineBuilder skinnedMeshPipelineBuilder = CreateHDRPipelineBuilder();
	skinnedMeshPipelineBuilder.m_ShaderStages = shaderStages;
	skinnedMeshPipelineBuilder.m_VertexAttributeDescriptions = attributeDescriptions;
	skinnedMeshPipelineBuilder.m_VertexBindingDescriptions = bindingDescriptions;

	m_SkinnedMeshPipeline = skinnedMeshPipelineBuilder.CreatePipeline(m_Device, "Skinned Mesh Pipeline");

	bindingDescriptions =
	{
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::POSITION3D],
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::NORMAL],
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::UV],
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::COLOR],
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::TANGENT],
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::BITANGENT]
	};

	attributeDescriptions =
	{
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::POSITION3D],
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::NORMAL],
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::UV],
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::COLOR],
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::TANGENT],
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::BITANGENT]
	};

	ShaderModuleLoadInfo info2;
	info2.m_Path = "Assets/Shaders/StaticMesh.vert";
	info2.m_Stage = ShaderStage::VERTEX;
	vertex = CreateOrGetShaderModule(info2);

	info2.m_Path = "Assets/Shaders/StaticMesh.frag";
	info2.m_Stage = ShaderStage::FRAGMENT;
	fragment = CreateOrGetShaderModule(info2);

	GraphicsPipelineBuilder staticMeshPipelineBuilder = CreateHDRPipelineBuilder();
	shaderStages[0].module = vertex;
	shaderStages[1].module = fragment;
	staticMeshPipelineBuilder.m_ShaderStages = shaderStages;
	staticMeshPipelineBuilder.m_VertexAttributeDescriptions = attributeDescriptions;
	staticMeshPipelineBuilder.m_VertexBindingDescriptions = bindingDescriptions;

	m_StaticMeshPipeline = staticMeshPipelineBuilder.CreatePipeline(m_Device, "Static Mesh Pipeline");

	staticMeshPipelineBuilder.m_DepthStencilInfo.depthTestEnable = true;
	staticMeshPipelineBuilder.m_DepthStencilInfo.depthWriteEnable = false;
	staticMeshPipelineBuilder.m_RasterizationInfo.cullMode = VK_CULL_MODE_NONE;

	m_TranslucentStaticMeshPipeline = staticMeshPipelineBuilder.CreatePipeline(m_Device, "Translucent Static Mesh Pipeline");

	VkLabelObject(m_Device, VK_OBJECT_TYPE_PIPELINE_LAYOUT, m_HDRPipelineLayout, "HDR Pipeline Layout");
}

void PBE::Renderer::CreateTransferFence()
{
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkErrorCheck(vkCreateFence(m_Device, &fenceInfo, nullptr, &m_TransferFence));
	VkLabelObject(m_Device, VK_OBJECT_TYPE_FENCE, m_TransferFence, "Transfer Fence");
}

PBE::GraphicsPipelineBuilder PBE::Renderer::CreateCompositingPassPipelineBuilder()
{
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
		.width = static_cast<float>(m_SwapchainExtent.width),
		.height = static_cast<float>(m_SwapchainExtent.height)
	};

	VkRect2D scissor
	{
		.offset = { 0, 0 },
		.extent = m_SwapchainExtent
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
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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
	depthStencil.depthTestEnable = VK_FALSE;
	depthStencil.depthWriteEnable = VK_FALSE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH,
		VK_DYNAMIC_STATE_SCISSOR
	};

	GraphicsPipelineBuilder compositeBuilder{};
	compositeBuilder.m_ColorBlendInfo = colorBlending;
	compositeBuilder.m_ColorBlendAttachments = { colorBlendAttachment };
	compositeBuilder.m_DepthStencilInfo = depthStencil;
	compositeBuilder.m_InputAssemblyInfo = inputAssembly;
	compositeBuilder.m_MultisampleInfo = multisampling;
	compositeBuilder.m_RasterizationInfo = rasterizer;
	compositeBuilder.m_ViewportInfo = viewportState;
	compositeBuilder.m_DynamicStates = dynamicStates;
	compositeBuilder.m_ShaderStages = {}; // You should fill this in when assigning shaders.
	compositeBuilder.m_PipelineLayout = m_CompositingPipelineLayout;
	compositeBuilder.m_RenderPass = m_PostProcessRenderPass;
	compositeBuilder.m_SubpassIndex = 0;

	return compositeBuilder;
}


PBE::GraphicsPipelineBuilder PBE::Renderer::CreateHDRPipelineBuilder()
{
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
		.width = static_cast<float>(m_SwapchainExtent.width),
		.height = static_cast<float>(m_SwapchainExtent.height)
	};

	VkRect2D scissor
	{
		.offset = { 0, 0 },
		.extent = m_SwapchainExtent
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
	multisampling.rasterizationSamples = m_MsaaSamples;
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
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;

	std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineLayoutCreateInfo hdrLayoutInfo{};
	hdrLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	hdrLayoutInfo.setLayoutCount = 1;
	hdrLayoutInfo.pSetLayouts = &Material::s_DescriptorSetLayout;
	hdrLayoutInfo.pushConstantRangeCount = 0;
	hdrLayoutInfo.pPushConstantRanges = nullptr;

	ShaderModuleLoadInfo vertInfo, fragInfo;
	vertInfo.m_Path = "Assets/Shaders/StaticMesh.vert";
	vertInfo.m_Stage = ShaderStage::VERTEX;

	fragInfo.m_Path = "Assets/Shaders/StaticMesh.frag";
	fragInfo.m_Stage = ShaderStage::FRAGMENT;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);
	shaderStages[0].pName = "main";
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = CreateOrGetShaderModule(vertInfo);

	shaderStages[1].pName = "main";
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = CreateOrGetShaderModule(fragInfo);

	GraphicsPipelineBuilder hdrBuilder{};
	hdrBuilder.m_ColorBlendInfo = colorBlending;
	hdrBuilder.m_DepthStencilInfo = depthStencil;
	hdrBuilder.m_InputAssemblyInfo = inputAssembly;
	hdrBuilder.m_MultisampleInfo = multisampling;
	hdrBuilder.m_RasterizationInfo = rasterizer;
	hdrBuilder.m_ViewportInfo = viewportState;
	hdrBuilder.m_DynamicStates = dynamicStates;
	hdrBuilder.m_PipelineLayout = m_HDRPipelineLayout;
	hdrBuilder.m_RenderPass = m_HDRRenderPass;
	hdrBuilder.m_ColorBlendAttachments = { colorBlendAttachment };
	hdrBuilder.m_ShaderStages = shaderStages;
	hdrBuilder.m_SubpassIndex = 0;

	return hdrBuilder;
}

void PBE::Renderer::CreateDescriptorPool()
{
	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 2048 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2048 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2048 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2048 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 2048 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 2048 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2048 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2048 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2048 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 2048 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2048 }
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000;
	poolInfo.poolSizeCount = std::size(poolSizes);
	poolInfo.pPoolSizes = poolSizes;

	VkErrorCheck(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool));
	VkLabelObject(m_Device, VK_OBJECT_TYPE_DESCRIPTOR_POOL, m_DescriptorPool, "Descriptor Pool");
}

void PBE::Renderer::CreateDefaultWhiteTexture()
{
	Texture2D_T* texture = new Texture2D_T();
	texture->m_Width = 1;
	texture->m_Height = 1;
	texture->m_MipLevels = 1;
	texture->m_LayerCount = 1;
	texture->m_Format = VK_FORMAT_R8G8B8A8_UNORM;

	// Create the image
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = 1;
	imageInfo.extent.height = 1;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkErrorCheck(vmaCreateImage(m_Allocator, &imageInfo, &allocInfo, &texture->m_Image, &texture->m_Allocation, nullptr));

	// Create the image view
	VkImageViewCreateInfo imageView{};
	imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageView.image = texture->m_Image;
	imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageView.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageView.subresourceRange.baseMipLevel = 0;
	imageView.subresourceRange.levelCount = 1;
	imageView.subresourceRange.baseArrayLayer = 0;
	imageView.subresourceRange.layerCount = 1;

	VkErrorCheck(vkCreateImageView(m_Device, &imageView, nullptr, &texture->m_ImageView));

	// Label the texture
	VkLabelObject(m_Device, VK_OBJECT_TYPE_IMAGE, texture->m_Image, "Default White Texture");

	// Create a staging buffer to fill the texture
	uint8_t whiteData[4] = { 255, 255, 255, 255 }; // White color

	if (m_StagingBuffer.m_Size < sizeof(whiteData))
	{
		DestroyGPUBuffer(m_Allocator, m_StagingBuffer);

		m_StagingBuffer.m_Size = sizeof(whiteData);
		m_StagingBuffer = CreateGPUBuffer(
			m_Allocator,
			sizeof(whiteData),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_ONLY
		);

		VkLabelObject(m_Device, VK_OBJECT_TYPE_BUFFER, m_StagingBuffer.m_Buffer, "Staging Buffer");
		vmaSetAllocationName(m_Allocator, m_StagingBuffer.m_Allocation, "Staging Buffer Allocation");
	}

	// Copy the white color to the staging buffer
	void* data;
	vmaMapMemory(m_Allocator, m_StagingBuffer.m_Allocation, &data);
	memcpy(data, whiteData, sizeof(whiteData));
	vmaUnmapMemory(m_Allocator, m_StagingBuffer.m_Allocation);

	// Transition the image layout to TRANSFER_DST_OPTIMAL
	TransitionImageLayout(m_Device, m_CommandPool, m_GraphicsQueue, texture->m_Image, texture->m_Format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Copy data from the staging buffer to the image
	CopyBufferToImage(m_Device, m_CommandPool, m_GraphicsQueue, m_StagingBuffer.m_Buffer, sizeof(whiteData), texture->m_Image, texture->m_Width, texture->m_Height);

	// Transition the image layout to SHADER_READ_ONLY_OPTIMAL
	TransitionImageLayout(m_Device, m_CommandPool, m_GraphicsQueue, texture->m_Image, texture->m_Format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	texture->m_Sampler = m_Samplers[static_cast<size_t>(SamplerType::LINEAR_CLAMP)];
	m_DefaultWhiteTexture = texture;
}

void PBE::Renderer::CreateSamplers()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	//Don't change mipmap mode from LINEAR, nearest looks weird because it "clips" from one mip to the next
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VkErrorCheck(vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Samplers[static_cast<size_t>(SamplerType::LINEAR_CLAMP)]));

	VkLabelObject(m_Device, VK_OBJECT_TYPE_SAMPLER, m_Samplers[static_cast<size_t>(SamplerType::LINEAR_CLAMP)], "Linear Clamp Sampler");

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	VkErrorCheck(vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Samplers[static_cast<size_t>(SamplerType::LINEAR_REPEAT)]));

	VkLabelObject(m_Device, VK_OBJECT_TYPE_SAMPLER, m_Samplers[static_cast<size_t>(SamplerType::LINEAR_REPEAT)], "Linear Repeat Sampler");

	samplerInfo.magFilter = VK_FILTER_NEAREST;
	samplerInfo.minFilter = VK_FILTER_NEAREST;

	VkErrorCheck(vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Samplers[static_cast<size_t>(SamplerType::NEAREST_REPEAT)]));

	VkLabelObject(m_Device, VK_OBJECT_TYPE_SAMPLER, m_Samplers[static_cast<size_t>(SamplerType::NEAREST_REPEAT)], "Nearest Repeat Sampler");

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

	VkErrorCheck(vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Samplers[static_cast<size_t>(SamplerType::NEAREST_CLAMP)]));

	VkLabelObject(m_Device, VK_OBJECT_TYPE_SAMPLER, m_Samplers[static_cast<size_t>(SamplerType::NEAREST_CLAMP)], "Nearest Clamp Sampler");


}

void PBE::Renderer::CreateDefaultMaterial()
{
	MaterialCreateInfo info{};
	
	info.m_Allocator = m_Allocator;
	info.m_Device = m_Device;
	info.m_DiffuseTexture = m_DefaultWhiteTexture;
	info.m_NormalTexture = m_DefaultWhiteTexture;
	info.m_ParallaxTexture = m_DefaultWhiteTexture;
	info.m_SpecularGlossEmitTexture = m_DefaultWhiteTexture;

	m_DefaultMaterial = CreateMaterial(info);
}

void PBE::Renderer::CreateBloomPipeline()
{
	// Descriptor layout
	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.pImmutableSamplers = &m_Samplers[static_cast<size_t>(SamplerType::NEAREST_CLAMP)];
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo dsInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	dsInfo.bindingCount = 1;
	dsInfo.pBindings = &samplerLayoutBinding;
	VkErrorCheck(vkCreateDescriptorSetLayout(m_Device, &dsInfo, nullptr, &m_BloomDescriptorSetLayout));
	VkLabelObject(m_Device, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, m_BloomDescriptorSetLayout, "Bloom Descriptor Set Layout");

	// Shader stages
	VkShaderModule vert = PBE::CreateOrGetShaderModule(m_Device, glslang_stage_t::GLSLANG_STAGE_VERTEX, "Assets/Shaders/bloom.vert");
	VkShaderModule frag = PBE::CreateOrGetShaderModule(m_Device, glslang_stage_t::GLSLANG_STAGE_FRAGMENT, "Assets/Shaders/bloom.frag");

	VkPipelineShaderStageCreateInfo stages[2]{};

	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vert;
	stages[0].pName = "main";

	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = frag;
	stages[1].pName = "main";

	VkPipelineVertexInputStateCreateInfo vertexInput{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	// No vertex buffers; fullscreen triangle in VS

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Dynamic viewport & scissor
	static std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	viewportState.pViewports = nullptr; // dynamic
	viewportState.pScissors = nullptr;  // dynamic

	VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.sampleShadingEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	VkPipelineLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	layoutInfo.setLayoutCount = 1;
	layoutInfo.pSetLayouts = &m_BloomDescriptorSetLayout;
	layoutInfo.pushConstantRangeCount = 0;
	VkErrorCheck(vkCreatePipelineLayout(m_Device, &layoutInfo, nullptr, &m_BloomPipelineLayout));

	VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = stages;
	pipelineInfo.pVertexInputState = &vertexInput;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_BloomPipelineLayout;
	pipelineInfo.renderPass = m_BloomRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	VkPipeline pipeline;
	VkErrorCheck(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));
	m_BloomPipeline = pipeline;
	VkLabelObject(m_Device, VK_OBJECT_TYPE_PIPELINE, m_BloomPipeline, "Bloom Pipeline");

	vkDestroyShaderModule(m_Device, vert, nullptr);
	vkDestroyShaderModule(m_Device, frag, nullptr);
}

void PBE::Renderer::CreateUIMaterial()
{
	MaterialCreateInfo info{};
	info.m_Allocator = m_Allocator;
	info.m_Device = m_Device;
	info.m_DiffuseTexture = m_DefaultWhiteTexture;
	info.m_NormalTexture = m_DefaultWhiteTexture;
	info.m_ParallaxTexture = m_DefaultWhiteTexture;
	info.m_SpecularGlossEmitTexture = m_DefaultWhiteTexture;

	m_UIMaterial = CreateMaterial(info);
}

void PBE::Renderer::CreateUIPipeline()
{

	std::array<VkDescriptorSetLayout, 3> layouts = { Material::s_DescriptorSetLayout , Camera::s_DescriptorSetLayout, Widget::s_DescriptorSetLayout };
	std::array<VkPushConstantRange, 1> pushConstants = {  };

	pushConstants[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
	pushConstants[0].offset = 0;
	pushConstants[0].size = sizeof(CameraModelPushConstantData);

	VkPipelineLayoutCreateInfo uiLayoutInfo{};
	uiLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	uiLayoutInfo.setLayoutCount = (uint32_t)layouts.size();
	uiLayoutInfo.pSetLayouts = layouts.data();
	uiLayoutInfo.pushConstantRangeCount = (uint32_t)pushConstants.size();
	uiLayoutInfo.pPushConstantRanges = pushConstants.data();

	VkErrorCheck(vkCreatePipelineLayout(m_Device, &uiLayoutInfo, nullptr, &m_UIPipelineLayout));
	VkLabelObject(m_Device, VK_OBJECT_TYPE_PIPELINE_LAYOUT,m_UIPipelineLayout, "UI Pipeline Layout");

	ShaderModuleLoadInfo info;
	info.m_Path = "Assets/Shaders/ui.vert";
	info.m_Stage = ShaderStage::VERTEX;

	VkShaderModule vertex, fragment;
	vertex = CreateOrGetShaderModule(info);

	ShaderModuleLoadInfo info2;
	info2.m_Path = "Assets/Shaders/ui.frag";
	info2.m_Stage = ShaderStage::FRAGMENT;

	fragment = CreateOrGetShaderModule(info2);

	VkPipelineVertexInputStateCreateInfo vertexInfo{};
	vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	std::vector<VkVertexInputBindingDescription> bindingDescriptions =
	{
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::POSITION3D],
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::UV],
		GPUMesh::s_VertexBindingDescription[(size_t)VertexAttribute::COLOR]
	};

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions =
	{
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::POSITION3D],
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::UV],
		GPUMesh::s_VertexAttributeDescriptions[(size_t)VertexAttribute::COLOR]
	};

	vertexInfo.vertexBindingDescriptionCount = (uint32_t)bindingDescriptions.size();
	vertexInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
	vertexInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);

	shaderStages[0].module = vertex;
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].pName = "main";


	shaderStages[1].module = fragment;
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].pName = "main";

	GraphicsPipelineBuilder uiPipelineBuilder = CreateCompositingPassPipelineBuilder();
	uiPipelineBuilder.m_PipelineLayout = m_UIPipelineLayout;
	uiPipelineBuilder.m_RenderPass = m_PostProcessRenderPass;
	uiPipelineBuilder.m_ShaderStages = shaderStages;
	uiPipelineBuilder.m_VertexAttributeDescriptions = attributeDescriptions;
	uiPipelineBuilder.m_VertexBindingDescriptions = bindingDescriptions;
	uiPipelineBuilder.m_SubpassIndex = 0;

	m_UISubpassIndex = 0;
	m_UIPipeline = uiPipelineBuilder.CreatePipeline(m_Device, "UI Pipeline");
}

void PBE::Renderer::CreateSkyboxPipeline()
{
}

void PBE::Renderer::CreateCSMResources()
{
}

void PBE::Renderer::CreateRenderDescriptorSetLayout()
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;

	VkErrorCheck(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool));

	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	VkErrorCheck(vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_RenderDescriptorSetLayout));

	VkLabelObject(m_Device, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, m_RenderDescriptorSetLayout, "Render Descriptor Set Layout");
}

void PBE::Renderer::CreateRenderDescriptorSet()
{
	for (size_t i = 0; i < m_RenderConstantBuffers.size(); ++i)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(RendererConstants);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		m_RenderConstantBuffers[i] = CreateGPUBuffer(m_Allocator, sizeof(RendererConstants), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	}

	std::array<VkDescriptorSetLayout, MAX_FRAMES_IN_FLIGHT> layouts 
	{ m_RenderDescriptorSetLayout };

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.descriptorSetCount = (uint32_t)layouts.size();
	allocInfo.pSetLayouts = layouts.data();

	VkErrorCheck(vkAllocateDescriptorSets(m_Device, &allocInfo, m_RenderDescriptorSet.data()));

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_RenderConstantBuffers[i].m_Buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(RendererConstants);

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_RenderDescriptorSet[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(m_Device, 1, &descriptorWrite, 0, nullptr);
	}
}

void PBE::Renderer::CreateImGuiRenderPass()
{
}

void PBE::Renderer::CreateCSMRenderPass()
{
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = FindDepthFormat(m_PhysicalDevice);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 0;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 0;
	subpass.pColorAttachments = nullptr;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &depthAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	//VkErrorCheck(vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_CSMRenderPass));
	//VkLabelObject(m_Device, VK_OBJECT_TYPE_RENDER_PASS, m_CSMRenderPass, "CSD Render Pass");
}

void PBE::Renderer::CreateCompositingPipeline()
{
	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = &m_Samplers[static_cast<size_t>(SamplerType::LINEAR_CLAMP)];
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding bloomArrayBinding{};
	bloomArrayBinding.binding = 1;
	bloomArrayBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bloomArrayBinding.descriptorCount = m_BloomPassCount; // N bloom levels
	bloomArrayBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bloomArrayBinding.pImmutableSamplers = VK_NULL_HANDLE;

	std::vector<VkDescriptorSetLayoutBinding> bindings = { samplerLayoutBinding, bloomArrayBinding };

	VkDescriptorSetLayoutCreateInfo samplerLayoutInfo{};
	samplerLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	samplerLayoutInfo.bindingCount = bindings.size();
	samplerLayoutInfo.pBindings = bindings.data();

	VkErrorCheck(vkCreateDescriptorSetLayout(m_Device, &samplerLayoutInfo, nullptr, &m_CompositingDescriptorSetLayout));
	VkLabelObject(m_Device, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, m_CompositingDescriptorSetLayout, "Compositing Descriptor Set Layout");

	VkShaderModule vertex, fragment;
	ShaderModuleLoadInfo loadInfo{};
	loadInfo.m_Path = "Assets/Shaders/tonemap.vert";
	loadInfo.m_Stage = ShaderStage::VERTEX;
	vertex = CreateOrGetShaderModule(loadInfo);

	loadInfo.m_Path = "Assets/Shaders/tonemap.frag";
	loadInfo.m_Stage = ShaderStage::FRAGMENT;
	fragment = CreateOrGetShaderModule(loadInfo);

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	shaderStages.resize(2);

	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = vertex;
	shaderStages[0].pName = "main";

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = fragment;
	shaderStages[1].pName = "main";

	VkPipelineLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = 1;
	layoutInfo.pSetLayouts = &m_CompositingDescriptorSetLayout;
	layoutInfo.pushConstantRangeCount = 0;

	VkErrorCheck(vkCreatePipelineLayout(m_Device, &layoutInfo, nullptr, &m_CompositingPipelineLayout));

	GraphicsPipelineBuilder compositeBuilder = CreateCompositingPassPipelineBuilder();
	compositeBuilder.m_ShaderStages = shaderStages;

	m_CompositingPipeline = compositeBuilder.CreatePipeline(m_Device, "Compositing Pipeline");
}

void PBE::Renderer::CreatePostProcessRenderPass()
{
	VkAttachmentDescription colorAttachment
	{
		.format = GetSwapchainSurfaceFormat(m_PhysicalDevice, m_Surface).format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentReference colorAttachmentRef
	{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass
	{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
	};

	VkSubpassDependency dependency
	{
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dependencyFlags = 0,
	};

	std::array<VkAttachmentDescription, 1> attachments = { colorAttachment };
	std::array<VkSubpassDescription, 1> subpasses = { subpass };
	std::array<VkSubpassDependency, 1> dependencies = { dependency };

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = (uint32_t)attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = (uint32_t)subpasses.size();
	renderPassInfo.pSubpasses = subpasses.data();
	renderPassInfo.dependencyCount = (uint32_t)dependencies.size();
	renderPassInfo.pDependencies = dependencies.data();

	VkErrorCheck(vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_PostProcessRenderPass));
	VkLabelObject(m_Device, VK_OBJECT_TYPE_RENDER_PASS, m_PostProcessRenderPass, "Compositing Renderpass");
}

void PBE::Renderer::CreateCompositingDescriptorResources()
{
	VkDescriptorSetLayout layouts[] = { m_CompositingDescriptorSetLayout };

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	VkErrorCheck(vkAllocateDescriptorSets(m_Device, &allocInfo, &m_CompositingDescriptorSet));
	VkLabelObject(m_Device, VK_OBJECT_TYPE_DESCRIPTOR_SET, m_CompositingDescriptorSet, "Compositing Descriptor Set");

	// --- Binding 0: Scene HDR Texture ---
	VkDescriptorImageInfo sceneImageInfo{};
	sceneImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	sceneImageInfo.imageView = m_HDRResolveImageView;
	sceneImageInfo.sampler = m_Samplers[static_cast<size_t>(SamplerType::LINEAR_CLAMP)];

	// --- Binding 1: Array of Bloom Mip ImageInfos ---
	std::vector<VkDescriptorImageInfo> bloomImageInfos;
	bloomImageInfos.reserve(m_BloomImageViews.size());

	for (VkImageView view : m_BloomImageViews)
	{
		VkDescriptorImageInfo info{};
		info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		info.imageView = view;
		info.sampler = m_Samplers[static_cast<size_t>(SamplerType::LINEAR_CLAMP)];
		bloomImageInfos.push_back(info);
	}

	// --- Descriptor Writes ---
	std::array<VkWriteDescriptorSet, 1> writes{};

	// Binding 0 - Scene HDR
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet = m_CompositingDescriptorSet;
	writes[0].dstBinding = 0;
	writes[0].dstArrayElement = 0;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writes[0].descriptorCount = 1;
	writes[0].pImageInfo = &sceneImageInfo;

	vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void PBE::Renderer::CreateSwapchainFramebuffers()
{
	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = m_PostProcessRenderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.width = m_SwapchainExtent.width;
	framebufferInfo.height = m_SwapchainExtent.height;
	framebufferInfo.layers = 1;

	m_SwapchainFramebuffers.resize(m_SwapchainImageViews.size());
	for (size_t i = 0; i < m_SwapchainImageViews.size(); i++)
	{
		framebufferInfo.pAttachments = &m_SwapchainImageViews[i];
		VkErrorCheck(vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_SwapchainFramebuffers[i]));
		std::string label = "Swapchain Framebuffer " + std::to_string(i);
		VkLabelObject(m_Device, VK_OBJECT_TYPE_FRAMEBUFFER, m_SwapchainFramebuffers[i], label);
	}
}

void PBE::Renderer::DestroySwapchainResources()
{
	for (VkFramebuffer framebuffer : m_SwapchainFramebuffers)
	{
		vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
	}

	for (VkImageView imageView : m_SwapchainImageViews)
	{
		vkDestroyImageView(m_Device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
}

void PBE::Renderer::CreateSwapchainResources()
{
	auto swapchainSupport = QuerySwapchainSupport(m_PhysicalDevice, m_Surface);

	int windowWidth, windowHeight;
	m_Info.m_pWindow->GetFramebufferSize(&windowWidth, &windowHeight);
	VkExtent2D windowExtent = { static_cast<uint32_t>(windowWidth), static_cast<uint32_t>(windowHeight) };

	uint32_t imageCount;
	m_Swapchain = CreateSwapchain(
		m_PhysicalDevice,
		m_Device,
		m_Surface,
		windowExtent,
		m_GraphicsQueueIndex,
		m_PresentQueueIndex,
		m_ComputeQueueIndex,
		&m_SwapchainImageFormat,
		&m_SwapchainExtent,
		&imageCount
	);

	// Get the swapchain images. No need to cleanup as the swapchain will do that for us.
	m_SwapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, m_SwapchainImages.data());
	m_SwapchainImageViews = CreateSwapchainImageViews(m_Device, m_SwapchainImages.data(), imageCount, m_SwapchainImageFormat);
	CreateSwapchainFramebuffers();

	for (int i = 0; i < m_SwapchainImages.size(); ++i)
	{
		std::string indexLabel = std::to_string(i);

		VkLabelObject(m_Device, VK_OBJECT_TYPE_IMAGE, m_SwapchainImages[i], std::string("Swapchain Image ") + indexLabel);
		VkLabelObject(m_Device, VK_OBJECT_TYPE_IMAGE_VIEW, m_SwapchainImageViews[i], std::string("Swapchain Image View ") + indexLabel);
	}

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		std::string indexLabel = std::to_string(i);
		VkLabelObject(m_Device, VK_OBJECT_TYPE_FRAMEBUFFER, m_SwapchainFramebuffers[i], std::string("Swapchain Framebuffer ") + indexLabel);
	}

	VkLabelObject(m_Device, VK_OBJECT_TYPE_SWAPCHAIN_KHR, m_Swapchain, "Swapchain");
}

void PBE::Renderer::DestroyHDRResources()
{
	vkDestroyImageView(m_Device, m_HDRImageView, nullptr);
	vmaDestroyImage(m_Allocator, m_HDRImage, m_HDRImageAllocation);

	vkDestroyImageView(m_Device, m_DepthImageView, nullptr);
	vmaDestroyImage(m_Allocator, m_DepthImage, m_DepthImageAllocation);

	vkDestroyImageView(m_Device, m_HDRResolveImageView, nullptr);
	vmaDestroyImage(m_Allocator, m_HDRResolveImage, m_HDRResolveImageAllocation);

	vkDestroyRenderPass(m_Device, m_HDRRenderPass, nullptr);
	vkDestroyFramebuffer(m_Device, m_HDRFramebuffer, nullptr);
}

void PBE::Renderer::RecreateScreenResources()
{
	vkDeviceWaitIdle(m_Device);
	DestroySwapchainResources();
	DestroyHDRResources();

	CreateSwapchainResources();
	CreateHDRResources();
	RecreateBloomScreenResources();
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = m_HDRResolveImageView;
	imageInfo.sampler = m_Samplers[static_cast<size_t>(SamplerType::LINEAR_CLAMP)];

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = m_CompositingDescriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(m_Device, 1, &descriptorWrite, 0, nullptr);
}

void PBE::Renderer::RecordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex, uint32_t frameIndex)
{
	//memcpy(m_RenderConstantBuffers[frameIndex].m_Data, &m_RenderConstants, sizeof(RendererConstants));
	//vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_HDRPipelineLayout, 2, 1, &m_RenderDescriptorSet[frameIndex], 0, nullptr);

	// Cleanup the staging buffers. They are garbage if we are on the frame they were used on.
	for (GPUBuffer& stagingBuffer : m_InFlightStagingBuffers[frameIndex])
	{
		DestroyGPUBuffer(m_Allocator, stagingBuffer);
	}
	m_InFlightStagingBuffers[frameIndex].clear();
	for (GPUBuffer& buffer : m_BuffersToDestroy)
	{
		DestroyGPUBuffer(m_Allocator, buffer);
	}
	m_BuffersToDestroy.clear();

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkErrorCheck(vkBeginCommandBuffer(cmd, &beginInfo));

	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	VkBeginDebugUtilsLabel(m_Device, cmd, "Before Renderpass", clearColor);
	{
		m_TransferEvent.Invoke(this, cmd, frameIndex);
	};
	VkEndDebugUtilsLabel(m_Device, cmd);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_SwapchainExtent.width);
	viewport.height = static_cast<float>(m_SwapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_SwapchainExtent;

	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	float labelColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };

	VkBeginDebugUtilsLabel(m_Device, cmd, "Before Tonemapper Renderpass", labelColor);
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_HDRRenderPass;
		renderPassInfo.framebuffer = m_HDRFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapchainExtent;

		VkClearValue clearColor2 = { 1.0f, 0.0f, 1.0f, 1.0f };
		VkClearValue clearDepth = { 1.0f, 0 };
		renderPassInfo.clearValueCount = 2;
		VkClearValue clearValues[] = { clearColor2, clearDepth };
		renderPassInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		m_DrawEvent.Invoke(this, cmd, frameIndex);
		vkCmdSetStencilTestEnable(cmd, VK_TRUE);
		vkCmdSetStencilWriteMask(cmd, VK_STENCIL_FACE_FRONT_AND_BACK, 0x01);
		m_TranslucentDrawEvent.Invoke(this, cmd, frameIndex);
		vkCmdSetStencilTestEnable(cmd, VK_FALSE);
		vkCmdEndRenderPass(cmd);
	};
	VkEndDebugUtilsLabel(m_Device,cmd);

	VkBeginDebugUtilsLabel(m_Device,cmd, "Post Process Renderpass", labelColor);
	{
		for (int i = 0; i < m_BloomImageViews.size(); ++i)
		{
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_BloomRenderPass;
			renderPassInfo.framebuffer = m_BloomFramebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_BloomExtents[i];

			VkClearValue clearColor3 = { 0.0f, 0.0f, 0.0f, 1.0f };

			renderPassInfo.clearValueCount = 1;
			VkClearValue clearValues[] = { clearColor3 };
			renderPassInfo.pClearValues = clearValues;

			vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_BloomPipeline);
			vkCmdBindDescriptorSets(
				cmd,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_BloomPipelineLayout,
				0,
				1,
				&m_BloomDescriptorSets[i],
				0,
				nullptr
			);
			VkViewport uiviewport{};
			uiviewport.x = 0.0f;
			uiviewport.y = 0.0f;
			uiviewport.width = static_cast<float>(m_BloomExtents[i].width);
			uiviewport.height = static_cast<float>(m_BloomExtents[i].height);
			uiviewport.minDepth = 0.0f;
			uiviewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmd, 0, 1, &uiviewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = m_BloomExtents[i];
			vkCmdSetScissor(cmd, 0, 1, &scissor);

			vkCmdDraw(cmd, 6, 1, 0, 0); // if using triangle technique

			vkCmdEndRenderPass(cmd);
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_PostProcessRenderPass;
		renderPassInfo.framebuffer = m_SwapchainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapchainExtent;

		VkClearValue clearColor3 = { 0.0f, 0.0f, 0.0f, 1.0f };

		renderPassInfo.clearValueCount = 1;
		VkClearValue clearValues[] = { clearColor3 };
		renderPassInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_CompositingPipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_CompositingPipelineLayout, 0, 1, &m_CompositingDescriptorSet, 0, nullptr);

		VkViewport uiviewport{};
		uiviewport.x = 0.0f;
		uiviewport.y = 0.0f;
		uiviewport.width = static_cast<float>(m_SwapchainExtent.width);
		uiviewport.height = static_cast<float>(m_SwapchainExtent.height);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_SwapchainExtent;
		vkCmdSetScissor(cmd, 0, 1, &scissor);
		float constants[4] = { 1.f, 0.04f, 1.f, 0.0f }; // Example constants for tonemapping
		vkCmdPushConstants(cmd, m_CompositingPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float) * 4, &constants);
		vkCmdSetViewport(cmd, 0, 1, &uiviewport);
		vkCmdDraw(cmd, 6, 1, 0, 0);

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_UIPipeline);
		if (m_UIMaterial)
		{
			m_UIMaterial->CmdBind(cmd, m_UIPipelineLayout, frameIndex);
		}
		m_PostProcessEvent.Invoke(this, cmd, frameIndex);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
		vkCmdEndRenderPass(cmd);
	};
	VkEndDebugUtilsLabel(m_Device,cmd);
	VkErrorCheck(vkEndCommandBuffer(cmd));
}
