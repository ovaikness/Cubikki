#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Mat4.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Frustum.hpp"

#include "Engine/Graphics/Renderer.hpp"
#include "Engine/Graphics/GraphicsUtils.hpp"
#include "Engine/Graphics/GPUBuffer.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <string>
#include <array>
#include <vulkan/vulkan.h>

struct VkCommandBuffer_T;
struct VkPipelineLayout_T;

typedef VkCommandBuffer_T *VkCommandBuffer;
typedef VkPipelineLayout_T *VkPipelineLayout;

namespace PBE
{
	enum FrustCorner
	{
		NEAR_BOTTOM_LEFT = 0,
		NEAR_BOTTOM_RIGHT,
		NEAR_TOP_LEFT,
		NEAR_TOP_RIGHT,
		FAR_BOTTOM_LEFT,
		FAR_BOTTOM_RIGHT,
		FAR_TOP_LEFT,
		FAR_TOP_RIGHT
	};
	enum class CameraType
	{
		PERSPECTIVE,
		ORTHOGRAPHIC
	};

	enum class ViewportMode
	{
		FIT,
		FULL,
		STRETCH
	};
	struct CameraConstants
	{
		alignas(16) Mat4 m_View;
		alignas(16) Mat4 m_Projection;
	};

	class Camera
	{
	public:
		static VkDescriptorPool s_CameraDescriptorPool;
		static VkDescriptorSetLayout s_DescriptorSetLayout;

		std::array<bool, MAX_FRAMES_IN_FLIGHT> m_Dirty { true };
		std::array<GPUBuffer, MAX_FRAMES_IN_FLIGHT> m_CameraBuffer;
		std::array<GPUBuffer, MAX_FRAMES_IN_FLIGHT> m_CameraBufferStaging;
		std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> m_DescriptorSets{ VK_NULL_HANDLE };

		ViewportMode m_ViewportMode{ ViewportMode::FIT };

		std::vector<AABB2> m_NormalizedViewports{ AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)) };
		std::vector<AABB2> m_RenderViewports{ AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)) };

		bool m_Enabled { true };
		Vec3 m_Position {0.f};
		EulerAngles m_Orientation { 0.f, 0.f , 0.f };
		Renderer* m_Renderer;
	public:
		Camera(Renderer* renderer);
		~Camera();

		static void CreateDescriptorSetLayout(VkDevice device, uint32_t cameraCount);
		static void DestroyDescriptorSetLayout(VkDevice device);

		float GetNear() const {
			return m_Near;
		}
		float GetFar() const {
			return m_Far;
		}
		float GetFOV() const {
			return m_FOV;
		}

		void OnResize(uint32_t width, uint32_t height);

		void CmdTransfer(VkCommandBuffer commandBuffer, uint32_t frameIndex);

		Mat4 GetViewMatrix() const;
		Mat4 GetProjectionMatrix() const;

		void SetViewMatrix(Mat4 const& view);
		void SetProjectionMatrix(Mat4 const& projection);

		void SetOrientation(EulerAngles const& orientation);
		void SetLocalPosition(Vec3 const& position);

		void CreatePerspective(float fovDegrees, float aspect, float nearZ, float farZ);
		void CreateOrthographic(AABB2 view, float nearZ, float farZ);

		void Translate(Vec3 const& translation);
		void Rotate(EulerAngles const& rotation);

		std::vector<PBE::Vec4> GetFrustumCorners() const;

		Mat4 GetCombinedMatrix() const;
		Frustum GetFrustum() const;

		std::array<PBE::Vec3, 8> GetCorners() const;
		
		Vec3 GetLocalPosition() const;

		Vec3 GetForward() const;
		Vec3 GetLeft() const;
		Vec3 GetUp() const;

		void MoveLeft(float distance);
		void MoveForward(float distance);
		void MoveUp(float distance);

		void CmdBind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t frameIndex, uint32_t setIndex = 2);
	private:
		friend class Renderer;
		CameraType m_Type{ CameraType::PERSPECTIVE };
		Mat4 m_View{};
		Mat4 m_Projection {};

		AABB2 m_NormalizedViewport{Vec2(0.f,0.f),Vec2(1.f,1.f)};
		Frustum m_Frustum;

		float m_Aspect{ 1.f };
		float m_FOV{ 45.f };
		float m_Near{ 0.1f };
		float m_Far{ 1000.f };

		void RecalculateViewFrustum();
		void MarkDirty();
		void Free(Renderer* renderer);
		Mat4 CalculateViewMatrix();
	};
}