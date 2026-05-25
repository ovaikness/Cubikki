#pragma once
#define VMA_DEBUG_LOG_FORMAT 1
#define VMA_LEAK_LOG_FORMAT 1
#include "Engine/Core/EventDispatcher.hpp"
#include "Engine/Graphics/Window.hpp"

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Mat4.hpp"

#include "Engine/Graphics/Rgba8.hpp"
#include "Engine/Graphics/Texture.hpp"
#include "Engine/Graphics/GPUBuffer.hpp"
#include "Engine/Graphics/GraphicsUtils.hpp"
#include "Engine/Graphics/Material.hpp"
#include "Engine/Graphics/Mesh.hpp"
#include "Engine/Graphics/PipelineBuilder.hpp"

#include <vulkan/vulkan.h>
#include "ThirdParty/vma/vk_mem_alloc.h"

#include <vector>
#include <array>
#include <map>
#include <filesystem>
#include <string>
#include <cstdint>

namespace PBE
{
	class Camera;
	class CPUMesh;
	class GPUMesh;

	enum class ShaderStage
	{
		VERTEX,
		GEOMETRY,
		FRAGMENT,
		COMPUTE,
		NUM_SHADER_STAGES
	};

	struct ShaderModuleLoadInfo
	{
		ShaderStage m_Stage{ ShaderStage::NUM_SHADER_STAGES };
		std::filesystem::path m_Path;

		bool operator<(ShaderModuleLoadInfo const& other) const
		{
			if (m_Stage < other.m_Stage)
				return true;
			if (m_Stage > other.m_Stage)
				return false;

			return m_Path < other.m_Path;
		}
	};

	struct TextureLoadInfo
	{
		std::filesystem::path m_Path;
		SamplerType m_SamplerType;
		TextureType m_TextureType;

		bool operator<(TextureLoadInfo const& other) const
		{

			size_t const hash = std::hash<std::string>{}(m_Path.string());
			size_t const otherHash = std::hash<std::string>{}(other.m_Path.string());

			if (hash < otherHash)
				return true;
			if (hash > otherHash)
				return false;

			if (m_TextureType < other.m_TextureType)
				return true;
			if (m_TextureType > other.m_TextureType)
				return false;

			return m_SamplerType < other.m_SamplerType;
		}
	};

	struct RendererCreateInfo
	{
		std::string ApplicationName;
		std::string EngineName;

		Window* m_pWindow = nullptr;
	};

	struct CameraModelPushConstantData
	{
		Mat4 m_Model;
		Mat4 m_Normal;
	};

	struct RendererConstants
	{
		Vec4 m_LightColor = Vec4(1.f, 1.f, 1.f, 1.f);
		Vec3 m_LightDirection = Vec3(1.f, 1.f, 1.f);
		float m_Padding;
	};

	class Renderer
	{
	public:
		bool m_BloomEnabled = true;
		VmaAllocator m_Allocator;
		VkDevice m_Device = VK_NULL_HANDLE;
		VkFence m_TransferFence = VK_NULL_HANDLE;

		VkDescriptorPool m_DescriptorPool;
		VkPipelineLayout m_HDRPipelineLayout;
		VkPipeline m_SkinnedMeshPipeline;
		VkPipeline m_StaticMeshPipeline;
		VkPipeline m_TranslucentStaticMeshPipeline;

		Material* m_UIMaterial;

		EventDispatcher<> m_PreRenderEvent;
		// Opaque Pass - Render all opaque objects (Renderer, VkCommandBuffer, FrameIndex)
		EventDispatcher<Renderer*, VkCommandBuffer, uint32_t> m_TransferEvent;
		EventDispatcher<Renderer*, VkCommandBuffer, uint32_t> m_DrawEvent;
		EventDispatcher<Renderer*, VkCommandBuffer, uint32_t> m_TranslucentDrawEvent;
		EventDispatcher<Renderer*, VkCommandBuffer, uint32_t> m_PostProcessEvent;
	public:
		Renderer(RendererCreateInfo const& info);
		Renderer(Renderer const&) = delete;
		Renderer& operator=(Renderer const&) = delete;
		~Renderer();
		
		VkDescriptorPool GetDescriptorPool() const 
		{
			return m_DescriptorPool;
		}
		Camera* CreateCamera();
		Material* CreateMaterial(MaterialCreateInfo const& info);
		std::shared_ptr<Material> CreateMaterialShared(MaterialCreateInfo const& info);

		// Single time command buffer dispatch to copy data to the GPU
		GPUMesh* CreateGPUMesh(CPUMesh const& cpuMesh, std::string_view debugName);
		GPUBuffer CmdCreateVertexBuffer(VkCommandBuffer cmd, uint32_t frameIndex, void const* data, size_t size, std::string_view debugName);
		GPUMesh* CmdCreateMesh(VkCommandBuffer cmd, uint32_t frameIndex, CPUMesh const& cpuMesh, std::string_view debugName);
		void AppendCopyBuffer(VkCommandBuffer cmd, uint32_t frameIndex, GPUBuffer buffer, void* pData, size_t size);

		// Dynamic mesh that can be updated every frame
		GPUMesh* CreateDynamicGPUMesh(CPUMesh const& cpuMesh, std::string_view debugName);

		void FreeGPUMesh(GPUMesh* mesh);

		void WaitIdle() const;

		Texture GetWhiteTexture() const { return m_DefaultWhiteTexture; }
		VkExtent2D GetSwapchainExtent() const { return m_SwapchainExtent; }
		VkPipelineLayout GetHDRPipelineLayout() const { return m_HDRPipelineLayout; }
		VkPipelineLayout GetUIPipelineLayout() const { return m_UIPipelineLayout; }
		VkPipelineLayout GetCompositePipelineLayout() const { return m_CompositingPipelineLayout; }

		VkShaderModule CreateOrGetShaderModule(ShaderModuleLoadInfo const& info);

		Texture2D LoadOrGetTexture2D(
			std::filesystem::path path,
			SamplerType type
		);

		Texture2D LoadTexture2D(
			std::filesystem::path path,
			SamplerType type
		);

		Texture2D LoadFromMemoryTexture2D(
			std::string_view name,
			VkExtent2D extent,
			void const* data,
			SamplerType type
		);

		Texture2D CreateTexture2D(
			std::string_view name,
			VkExtent2D extent, 
			VkFormat format, 
			VkImageTiling tiling, 
			VkImageUsageFlags usage, 
			VkSampleCountFlagBits samples,
			VmaMemoryUsage memoryUsage, 
			SamplerType type,
			VkImageAspectFlagBits aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT
		);

		Texture2DArray CreateTexture2DArray(
			std::string_view name,
			VkExtent3D extent,
			uint32_t layerCount,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkSampleCountFlagBits samples,
			VmaMemoryUsage memoryUsage,
			SamplerType type,
			VkImageAspectFlagBits aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT
			
		);

		void FreeTexture(Texture texture);

		PBE::GraphicsPipelineBuilder CreateCompositingPassPipelineBuilder();
		PBE::GraphicsPipelineBuilder CreateHDRPipelineBuilder();

		void BeginFrame();
		void SubmitCmdDraw();
	private:
		RendererCreateInfo m_Info;
		// Core
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		GPUBuffer m_ZeroBuffer;

		RendererConstants m_RenderConstants;
		std::array<GPUBuffer, MAX_FRAMES_IN_FLIGHT> m_RenderConstantBuffers;
		VkDescriptorSetLayout m_RenderDescriptorSetLayout;
		std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> m_RenderDescriptorSet;

		Material* m_DefaultMaterial;
		GPUMesh* m_ImmediateModeGPUMesh[MAX_FRAMES_IN_FLIGHT];
		GPUMesh* m_ImmediateModeUIGPUMesh[MAX_FRAMES_IN_FLIGHT];

		// Queues
		uint32_t m_GraphicsQueueIndex = 0;
		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;

		uint32_t m_PresentQueueIndex = 0;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;

		uint32_t m_ComputeQueueIndex = 0;
		VkQueue m_ComputeQueue = VK_NULL_HANDLE;

		// Swapchain
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		VkFormat m_SwapchainImageFormat;
		VkExtent2D m_SwapchainExtent = {};

		VkSampleCountFlagBits m_MsaaSamples = VK_SAMPLE_COUNT_1_BIT;
		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
		std::vector<VkFramebuffer> m_SwapchainFramebuffers;

		// Bloom Resources
		uint32_t m_BloomPassCount = 8;
		std::vector<VkDescriptorSet> m_BloomDescriptorSets;
		std::vector<VkImage> m_BloomImages;
		std::vector<VkImageView> m_BloomImageViews;
		std::vector<VmaAllocation> m_BloomAllocations;
		std::vector<VkExtent2D> m_BloomExtents;
		std::vector<VkFramebuffer> m_BloomFramebuffers;
		VkRenderPass m_BloomRenderPass;
		VkDescriptorSetLayout m_BloomDescriptorSetLayout;
		VkPipelineLayout m_BloomPipelineLayout;
		VkPipeline m_BloomPipeline;

		VkFormat m_HDRFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		VkFormat m_HDRResolveFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		VkImage m_HDRImage;
		VkImage m_HDRResolveImage;
		VmaAllocation m_HDRImageAllocation;
		VmaAllocation m_HDRResolveImageAllocation;
		VkImageView m_HDRImageView;
		VkImageView m_HDRStencilImageView;
		VkImageView m_HDRResolveImageView;

		VkDescriptorImageInfo m_HDRResolveDescriptor;

		VkFormat m_DepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
		VkImage m_DepthImage;
		VmaAllocation m_DepthImageAllocation;
		VkImageView m_DepthImageView;

		VkFramebuffer m_HDRFramebuffer;

		//Swapchain Synchronization
		uint32_t m_CurrentFrame = 0;
		std::vector<VkSemaphore> m_ImageAvailableSemaphore;
		std::vector<VkSemaphore> m_RenderFinishedSemaphore;
		std::vector<VkFence> m_InFlightFences;

		VkPipelineLayout m_SkyboxPipelineLayout;
		VkPipeline m_SkyboxPipeline;

		VkPipelineLayout m_UIPipelineLayout;
		VkPipeline m_UIPipeline;
		uint32_t m_UISubpassIndex = 0;

		//Render Passes
		VkRenderPass m_HDRRenderPass;
		uint32_t m_HDRSubpassIndex;

		VkRenderPass m_PostProcessRenderPass;
		uint32_t m_CompositingSubpassIndex;

		//Pipelines
		VkPipelineLayout m_CompositingPipelineLayout;
		VkPipeline m_CompositingPipeline;

		//Single descriptor set for compositing the HDR Image View (We need to bind the image view so we can sample it in the tonemap fragment shader)
		VkDescriptorSet m_CompositingDescriptorSet;
		VkDescriptorSetLayout m_CompositingDescriptorSetLayout;

		//Single descriptor set for binding usable vertex buffers and other renderer defined constants
		VkDescriptorSet m_VertexDescriptorSet;
		VkDescriptorSetLayout m_VertexDescriptorSetLayout;

		// Command Buffers
		VkCommandPool m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;
		
		VkRenderPass m_ImGuiRenderPass;

		//Samplers
		VkSampler m_Samplers[static_cast<size_t>(SamplerType::COUNT)];

		//Stop repeat texture loading
		std::map<ShaderModuleLoadInfo, VkShaderModule> m_LoadedShaderModules;
		std::map<TextureLoadInfo, Texture> m_LoadedTextures;
		std::vector<Texture> m_MemoryLoadedTextures;

		//Staging buffers
		GPUBuffer m_StagingBuffer{};

		//Host visible staging buffers for transferring data to the GPU
		//Should be used for transferring data to the GPU for frames in flight without stalling the CPU
		std::array<std::vector<GPUBuffer>,MAX_FRAMES_IN_FLIGHT> m_InFlightStagingBuffers;
		std::vector<GPUBuffer> m_BuffersToDestroy;
		//Default Vertex Buffers
		Texture m_DefaultWhiteTexture;
	private:
		void CreateTransferFence();

		void CreateDescriptorPool();
		void CreateDefaultWhiteTexture();
		void CreateSamplers();
		void CreateDefaultMaterial();
		void CreateBloomPipeline();
		void CreateUIMaterial();
		void CreateUIPipeline();
		void CreateSkyboxPipeline();
		void CreateCSMResources();

		void CreateRenderDescriptorSetLayout();
		void CreateRenderDescriptorSet();

		void CreateImGuiRenderPass();
		void CreateCSMRenderPass();

		void CreateHDRResources();
		void CreateBloomRenderPass();
		void CreateBloomResources();
		void CreateBloomScreenResources();
		void DestroyBloomScreenResources();
		void RecreateBloomScreenResources();
		void CreateHDRRenderPass();
		void CreateHDRFramebuffer();
		void CreateHDRPipeline();

		void CreateCompositingPipeline();
		void CreatePostProcessRenderPass();
		void CreateCompositingDescriptorResources();

		void CreateSwapchainFramebuffers();
		
		void DestroyHDRResources();

		void CreateSwapchainResources();
		void DestroySwapchainResources();

		void RecreateScreenResources();

		void RecordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex, uint32_t frameIndex);
	};
}