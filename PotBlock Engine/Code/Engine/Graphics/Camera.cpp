#include "Engine/Graphics/Camera.hpp"
#include "Engine/Graphics/Renderer.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Graphics/GraphicsUtils.hpp"

namespace PBE
{
	VkDescriptorPool Camera::s_CameraDescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSetLayout Camera::s_DescriptorSetLayout = VK_NULL_HANDLE;

	Camera::Camera(Renderer* renderer)
		: m_View(Mat4::IDENTITY), m_Projection(Mat4::IDENTITY), m_Position(0.f), m_Orientation(0.f, 0.f, 0.f), m_Renderer(renderer)
	{
		MarkDirty();

		std::array<VkDescriptorSetLayout, MAX_FRAMES_IN_FLIGHT> layouts;
		std::fill(layouts.begin(), layouts.end(), s_DescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = s_CameraDescriptorPool;
		allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
		allocInfo.pSetLayouts = layouts.data();

		std::vector<VkWriteDescriptorSet> descriptorWrites;
		VkErrorCheck(vkAllocateDescriptorSets(renderer->m_Device, &allocInfo, m_DescriptorSets.data()), "Failed to allocate camera descriptor sets!");

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VkLabelObject(renderer->m_Device, VK_OBJECT_TYPE_DESCRIPTOR_SET, m_DescriptorSets[i], Stringf("Camera Descriptor Set %i", i));

			CameraConstants cameraData = {};
			cameraData.m_View = m_View;
			cameraData.m_Projection = m_Projection;

			m_CameraBufferStaging[i] = CreateGPUBuffer(renderer->m_Allocator, sizeof(CameraConstants), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			m_CameraBuffer[i] = CreateGPUBuffer(renderer->m_Allocator, sizeof(CameraConstants), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

			m_CameraBuffer[i].m_Size = sizeof(CameraConstants);
			VkErrorCheck(vmaMapMemory(renderer->m_Allocator, m_CameraBufferStaging[i].m_Allocation, &m_CameraBufferStaging[i].m_Data), "Failed to map memory!");
			memcpy(m_CameraBufferStaging[i].m_Data, &cameraData, sizeof(CameraConstants));

			VkDescriptorBufferInfo m_CameraBufferInfo = {};
			m_CameraBufferInfo.buffer = m_CameraBuffer[i].m_Buffer;
			m_CameraBufferInfo.offset = 0;
			m_CameraBufferInfo.range = sizeof(CameraConstants);

			VkWriteDescriptorSet cameraDataWrite = {};
			cameraDataWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			cameraDataWrite.dstSet = m_DescriptorSets[i];
			cameraDataWrite.dstBinding = 0;
			cameraDataWrite.dstArrayElement = 0;
			cameraDataWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			cameraDataWrite.descriptorCount = 1;
			cameraDataWrite.pBufferInfo = &m_CameraBufferInfo;

			descriptorWrites.push_back(cameraDataWrite);
		}

		vkUpdateDescriptorSets(renderer->m_Device, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
	}

	Camera::~Camera()
	{
		Free(m_Renderer);
	}

	void Camera::CreateDescriptorSetLayout(VkDevice device, uint32_t cameraCount)
	{
		std::array<VkDescriptorPoolSize, 1> poolSizes = {};

		poolSizes[0] = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = cameraCount;

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = cameraCount;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		vkCreateDescriptorPool(device, &poolInfo, nullptr, &s_CameraDescriptorPool);

		std::array<VkDescriptorSetLayoutBinding, 1> layoutBindings = {};

		layoutBindings[0].binding = 0;
		layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[0].descriptorCount = 1;
		layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = (uint32_t)layoutBindings.size();
		layoutInfo.pBindings = layoutBindings.data();

		vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &s_DescriptorSetLayout);
	}

	void Camera::DestroyDescriptorSetLayout(VkDevice device)
	{
		vkDestroyDescriptorPool(device, s_CameraDescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device, s_DescriptorSetLayout, nullptr);
	}

	void Camera::OnResize(uint32_t width, uint32_t height)
	{
		m_RenderViewports.resize(m_NormalizedViewports.size());
		if (width == 0 || height == 0) return;
		if (m_ViewportMode == ViewportMode::FIT)
		{
			// Preserve aspect ratio
			float aspect = (float)width / ((float)height * 0.5f);
			float targetWidth, targetHeight;

			if (m_Aspect < aspect) // Viewport is narrower than target aspect
			{
				targetWidth = (float)height * m_Aspect;
				targetHeight = (float)height;
			}
			else // Viewport is wider than target aspect
			{
				targetWidth = (float)width;
				targetHeight = (float)width / m_Aspect;
			}

			// Calculate the fitted viewport area
			AABB2 fitFullViewport(
				Vec2((width - targetWidth) * 0.5f, (height - targetHeight) * 0.5f),
				Vec2(width - (width - targetWidth) * 0.5f, height - (height - targetHeight) * 0.5f)
			);
			 
			// Adjust each normalized viewport to fit within the new aspect-corrected area
			for (size_t i = 0; i < m_NormalizedViewports.size(); ++i)
			{
				AABB2& renderView = m_RenderViewports[i];

				Vec2 fitDimensions = fitFullViewport.GetDimensions();

				renderView.m_Mins.x = fitFullViewport.m_Mins.x + m_NormalizedViewports[i].m_Mins.x * fitDimensions.x;
				renderView.m_Mins.y = fitFullViewport.m_Mins.y + m_NormalizedViewports[i].m_Mins.y * fitDimensions.y;
				renderView.m_Maxs.x = fitFullViewport.m_Mins.x + m_NormalizedViewports[i].m_Maxs.x * fitDimensions.x;
				renderView.m_Maxs.y = fitFullViewport.m_Mins.y + m_NormalizedViewports[i].m_Maxs.y * fitDimensions.y;

				// Update the render viewport
				m_RenderViewports[i] = renderView;
			}
		}
		else
		{
			m_Aspect = (float)width / (float)height;

			for (size_t i = 0; i < m_NormalizedViewports.size(); ++i)
			{
				AABB2& renderView = m_RenderViewports[i];

				Vec2 fullDimensions = Vec2((float)width, (float)height);

				renderView.m_Mins.x = 0.f + m_NormalizedViewports[i].m_Mins.x * fullDimensions.x;
				renderView.m_Mins.y = 0.f + m_NormalizedViewports[i].m_Mins.y * fullDimensions.y;
				renderView.m_Maxs.x = 0.f + m_NormalizedViewports[i].m_Maxs.x * fullDimensions.x;
				renderView.m_Maxs.y = 0.f + m_NormalizedViewports[i].m_Maxs.y * fullDimensions.y;

				// Update the render viewport
				m_RenderViewports[i] = renderView;
			}
		}

		if (m_Type == CameraType::PERSPECTIVE)
		{
			CreatePerspective(m_FOV, m_Aspect, m_Near, m_Far);
		}

		MarkDirty();
	}

	void Camera::CmdTransfer(VkCommandBuffer commandBuffer, uint32_t frameIndex)
	{
		//if (!m_Dirty[frameIndex])
		//{
		//	return;
		//}
		//m_Dirty[frameIndex] = false;

		if (m_Type == CameraType::PERSPECTIVE)
		{
			RecalculateViewFrustum();
		}

		CameraConstants cameraData
		{
			.m_View = m_View,
			.m_Projection = m_Projection
		};

		memcpy(m_CameraBufferStaging[frameIndex].m_Data, &cameraData, sizeof(CameraConstants));

		VkBufferCopy copyRegion = {};
		copyRegion.dstOffset = 0;
		copyRegion.srcOffset = 0;
		copyRegion.size = sizeof(CameraConstants);

		vkCmdCopyBuffer(commandBuffer, m_CameraBufferStaging[frameIndex].m_Buffer, m_CameraBuffer[frameIndex].m_Buffer, 1, &copyRegion);

		VkDescriptorBufferInfo m_CameraBufferInfo = {};
		m_CameraBufferInfo.buffer = m_CameraBuffer[frameIndex].m_Buffer;
		m_CameraBufferInfo.offset = 0;
		m_CameraBufferInfo.range = sizeof(CameraConstants);

		VkWriteDescriptorSet cameraDataWrite = {};
		cameraDataWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cameraDataWrite.dstSet = m_DescriptorSets[frameIndex];
		cameraDataWrite.dstBinding = 0;
		cameraDataWrite.dstArrayElement = 0;
		cameraDataWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraDataWrite.descriptorCount = 1;
		cameraDataWrite.pBufferInfo = &m_CameraBufferInfo;

		vkUpdateDescriptorSets(m_Renderer->m_Device, 1, &cameraDataWrite, 0, nullptr);
	}

	PBE::Mat4 Camera::GetViewMatrix() const
	{
		return m_View;
	}

	PBE::Mat4 Camera::GetProjectionMatrix() const
	{
		return m_Projection;
	}

	void Camera::SetViewMatrix(Mat4 const& view)
	{
		m_View = view;
		MarkDirty();
	}

	void Camera::SetProjectionMatrix(Mat4 const& projection)
	{
		m_Projection = projection;
		MarkDirty();
	}

	void Camera::SetOrientation(EulerAngles const& orientation)
	{
		m_Orientation = orientation;
		m_View = CalculateViewMatrix();
		MarkDirty();
	}

	void Camera::SetLocalPosition(Vec3 const& position)
	{
		m_Position = position;
		m_View = CalculateViewMatrix();
		MarkDirty();
	}

	void Camera::CreatePerspective(float fovDegrees, float aspect, float nearZ, float farZ)
	{
		m_Type = CameraType::PERSPECTIVE;
		m_Aspect = aspect;
		m_Near = nearZ;
		m_Far = farZ;
		m_FOV = fovDegrees;
		Mat4 renderMatrix = Mat4(
			0, 0, 1, 0,
			-1, 0, 0, 0,
			0, -1, 0, 0,
			0, 0, 0, 1
		);

		m_Projection = renderMatrix.GetAppended(Mat4::CreatePerspectiveProjection(fovDegrees, aspect, nearZ, farZ));
		MarkDirty();
	}

	void Camera::CreateOrthographic(AABB2 view, float nearZ, float farZ)
	{
		m_Type = CameraType::ORTHOGRAPHIC;
		m_Projection = Mat4::CreateOrthographicProjection(view.m_Mins.x,view.m_Maxs.x, view.m_Mins.y,view.m_Maxs.y, nearZ, farZ);
		m_Aspect = view.GetDimensions().x / view.GetDimensions().y;

		m_RenderViewports.resize(m_NormalizedViewports.size());

		float width = view.GetDimensions().x;
		float height = view.GetDimensions().y;

		if (width == 0 || height == 0) return;
		if (m_ViewportMode == ViewportMode::FIT)
		{
			// Preserve aspect ratio
			float aspect = (float)width / ((float)height * 0.5f);
			float targetWidth, targetHeight;

			if (m_Aspect < aspect) // Viewport is narrower than target aspect
			{
				targetWidth = height * m_Aspect;
				targetHeight = height;
			}
			else // Viewport is wider than target aspect
			{
				targetWidth = width;
				targetHeight = width / m_Aspect;
			}

			// Calculate the fitted viewport area
			AABB2 fitFullViewport(
				Vec2((width - targetWidth) * 0.5f, (height - targetHeight) * 0.5f),
				Vec2(width - (width - targetWidth) * 0.5f, height - (height - targetHeight) * 0.5f)
			);

			// Adjust each normalized viewport to fit within the new aspect-corrected area
			for (size_t i = 0; i < m_NormalizedViewports.size(); ++i)
			{
				AABB2& renderView = m_RenderViewports[i];

				Vec2 fitDimensions = fitFullViewport.GetDimensions();

				renderView.m_Mins.x = fitFullViewport.m_Mins.x + m_NormalizedViewports[i].m_Mins.x * fitDimensions.x;
				renderView.m_Mins.y = fitFullViewport.m_Mins.y + m_NormalizedViewports[i].m_Mins.y * fitDimensions.y;
				renderView.m_Maxs.x = fitFullViewport.m_Mins.x + m_NormalizedViewports[i].m_Maxs.x * fitDimensions.x;
				renderView.m_Maxs.y = fitFullViewport.m_Mins.y + m_NormalizedViewports[i].m_Maxs.y * fitDimensions.y;

				// Update the render viewport
				m_RenderViewports[i] = renderView;
			}
		}
		else if (m_ViewportMode == ViewportMode::STRETCH)
		{
			m_Aspect = (float)width / ((float)height * 0.5f);

			AABB2 fullViewport(0.f, 0.f, width, height);

			for (size_t i = 0; i < m_NormalizedViewports.size(); ++i)
			{
				AABB2& renderView = m_RenderViewports[i];

				Vec2 fullDimensions = fullViewport.GetDimensions();

				renderView.m_Mins.x = fullViewport.m_Mins.x + m_NormalizedViewports[i].m_Mins.x * fullDimensions.x;
				renderView.m_Mins.y = fullViewport.m_Mins.y + m_NormalizedViewports[i].m_Mins.y * fullDimensions.y;
				renderView.m_Maxs.x = fullViewport.m_Mins.x + m_NormalizedViewports[i].m_Maxs.x * fullDimensions.x;
				renderView.m_Maxs.y = fullViewport.m_Mins.y + m_NormalizedViewports[i].m_Maxs.y * fullDimensions.y;

				// Update the render viewport
				m_RenderViewports[i] = renderView;
			}
		}

		MarkDirty();
	}

	void Camera::MoveLeft(float distance)
	{
		Vec3 left = m_View.GetJBasis3D();
		Translate(left * distance);
	}

	void Camera::MoveForward(float distance)
	{
		Vec3 forward = m_View.GetIBasis3D();
		Translate(forward * distance);
	}

	void Camera::MoveUp(float distance)
	{
		Vec3 up = m_View.GetKBasis3D();
		Translate(up * distance);
	}

	void Camera::Translate(Vec3 const& translation)
	{
		m_Position += translation;

		m_View = CalculateViewMatrix();
		MarkDirty();
	}

	void Camera::Rotate(EulerAngles const& rotation)
	{
		m_Orientation += rotation;

		m_View = CalculateViewMatrix();
		MarkDirty();
	}

	std::vector<PBE::Vec4> Camera::GetFrustumCorners() const
	{
		std::vector<PBE::Vec4> corners;
		Mat4 const inv = GetCombinedMatrix().GetInverse();
		for (int xx = -1; xx <= 1; xx += 2)
		{
			for (int yy = -1; yy <= 1; yy += 2)
			{
				for (int zz = 0; zz <= 1; zz += 1)
				{
					Vec4 corner = inv.TransformPoint(Vec4(static_cast<float>(xx), static_cast<float>(- yy), static_cast<float>(zz), 1.f));
					corner /= corner.w;
					corners.push_back(corner);
				}
			}
		}
		return corners;
	}

	Mat4 Camera::GetCombinedMatrix() const
	{
		return m_Projection.GetAppended(m_View);
	}

	Frustum Camera::GetFrustum() const
	{
		return m_Frustum;
	}

	std::array<PBE::Vec3, 8> Camera::GetCorners() const
	{
		Vec3 bl = Vec3(-1.f, -1.f, 0.f);
		Vec3 br = Vec3(1.f, -1.f, 0.f);
		Vec3 tl = Vec3(-1.f, 1.f, 0.f);
		Vec3 tr = Vec3(1.f, 1.f, 0.f);

		Vec3 fbl = Vec3(-1.f, -1.f, 1.f);
		Vec3 fbr = Vec3(1.f, -1.f, 1.f);
		Vec3 ftl = Vec3(-1.f, 1.f, 1.f);
		Vec3 ftr = Vec3(1.f, 1.f, 1.f);

		Mat4 inv = GetCombinedMatrix().GetInverse();

		bl = inv.TransformPoint(bl);
		br = inv.TransformPoint(br);
		tl = inv.TransformPoint(tl);
		tr = inv.TransformPoint(tr);

		fbl = inv.TransformPoint(fbl);
		fbr = inv.TransformPoint(fbr);
		ftl = inv.TransformPoint(ftl);
		ftr = inv.TransformPoint(ftr);

		return std::array<PBE::Vec3, 8>{bl, br, tl, tr, fbl, fbr, ftl, ftr};
	}

	PBE::Vec3 Camera::GetLocalPosition() const 
	{
		return m_Position;
	}

	PBE::Vec3 Camera::GetForward() const
	{
		Mat4 mat = m_View.GetTransposed();
		return mat.GetIBasis3D();
	}

	PBE::Vec3 Camera::GetLeft() const
	{
		Mat4 mat = m_View.GetTransposed();
		return mat.GetJBasis3D();
	}

	PBE::Vec3 Camera::GetUp() const
	{
		Mat4 mat = m_View.GetTransposed();
		return mat.GetKBasis3D();
	}

	Mat4 Camera::CalculateViewMatrix()
	{
		Mat4 mat = Mat4::CreateTranslation3D(-m_Position).GetAppended(m_Orientation.GetAsMatrix().GetTransposed());
		return mat;
	}

	void Camera::CmdBind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t frameIndex, uint32_t setIndex)
	{
		std::vector<VkViewport> viewports;
		for(int i = 0; i < m_RenderViewports.size(); ++i)
		{
			viewports.push_back({});
			viewports[i].x = m_RenderViewports[i].m_Mins.x;
			viewports[i].y = m_RenderViewports[i].m_Mins.y;
			viewports[i].width = m_RenderViewports[i].GetDimensions().x;
			viewports[i].height = m_RenderViewports[i].GetDimensions().y;
			viewports[i].minDepth = 0.f;
			viewports[i].maxDepth = 1.f;
		}

		vkCmdSetViewport(commandBuffer, 0, (uint32_t)viewports.size(), viewports.data());
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, setIndex, 1, &m_DescriptorSets[frameIndex], 0, nullptr);
	}

	void Camera::RecalculateViewFrustum()
	{
		if (m_Type == CameraType::ORTHOGRAPHIC)
		{
			return;
		}

		Vec3 nearCenter = m_Position + GetForward() * m_Near;
		Vec3 farCenter = m_Position + GetForward() * m_Far;

		float nearHeight = 2.f * TanDegrees(m_FOV * 0.5f) * m_Near;
		float nearWidth = nearHeight * m_Aspect;

		float farHeight = 2.f * TanDegrees(m_FOV * 0.5f) * m_Far;
		float farWidth = farHeight * m_Aspect;

		// Calculate the frustum corners
		Vec3 nearTopLeft = nearCenter + GetUp() * (nearHeight * 0.5f) + GetLeft() * (nearWidth * 0.5f);
		Vec3 nearTopRight = nearCenter + GetUp() * (nearHeight * 0.5f) - GetLeft() * (nearWidth * 0.5f);
		Vec3 nearBottomLeft = nearCenter - GetUp() * (nearHeight * 0.5f) + GetLeft() * (nearWidth * 0.5f);
		Vec3 nearBottomRight = nearCenter - GetUp() * (nearHeight * 0.5f) - GetLeft() * (nearWidth * 0.5f);

		Vec3 farTopLeft = farCenter + GetUp() * (farHeight * 0.5f) + GetLeft() * (farWidth * 0.5f);
		Vec3 farTopRight = farCenter + GetUp() * (farHeight * 0.5f) - GetLeft() * (farWidth * 0.5f);
		Vec3 farBottomLeft = farCenter - GetUp() * (farHeight * 0.5f) + GetLeft() * (farWidth * 0.5f);
		Vec3 farBottomRight = farCenter - GetUp() * (farHeight * 0.5f) - GetLeft() * (farWidth * 0.5f);

		// Normals for planes
		Vec3 nearNormal = GetForward();
		Vec3 farNormal = -GetForward();

		// Calculate the edge vectors and the corresponding plane normals
		Vec3 leftEdge0 = farBottomLeft - nearBottomLeft;
		Vec3 leftEdge1 = nearTopLeft - nearBottomLeft;
		Vec3 leftNormal = Vec3::CrossProduct(leftEdge0, leftEdge1).GetNormalized();

		Vec3 rightEdge0 = farBottomRight - nearBottomRight;
		Vec3 rightEdge1 = nearTopRight - nearBottomRight;
		Vec3 rightNormal = Vec3::CrossProduct(rightEdge1, rightEdge0).GetNormalized();

		Vec3 topEdge0 = farTopLeft - nearTopLeft;
		Vec3 topEdge1 = nearTopRight - nearTopLeft;
		Vec3 topNormal = Vec3::CrossProduct(topEdge0, topEdge1).GetNormalized();

		Vec3 bottomEdge0 = farBottomLeft - nearBottomLeft;
		Vec3 bottomEdge1 = nearBottomRight - nearBottomLeft;
		Vec3 bottomNormal = Vec3::CrossProduct(bottomEdge1, bottomEdge0).GetNormalized();

		// Initialize planes
		Plane nearP, farP, leftP, rightP, topP, bottomP;

		// Near plane (camera's near plane)
		nearP.m_Normal = nearNormal;
		nearP.m_Distance = Vec3::DotProduct(nearNormal, nearCenter); // Using nearCenter instead of m_Position for better accuracy

		// Far plane (camera's far plane)
		farP.m_Normal = farNormal;
		farP.m_Distance = Vec3::DotProduct(farNormal, farCenter); // Using farCenter to get the correct distance

		// Left plane
		leftP.m_Normal = leftNormal;
		leftP.m_Distance = Vec3::DotProduct(leftNormal, m_Position - GetForward() * 32.f);

		// Right plane
		rightP.m_Normal = rightNormal;
		rightP.m_Distance = Vec3::DotProduct(rightNormal, m_Position - GetForward() * 32.f);

		// Top plane
		topP.m_Normal = topNormal;
		topP.m_Distance = Vec3::DotProduct(topNormal, m_Position - GetForward() * 32.f);

		// Bottom plane
		bottomP.m_Normal = bottomNormal;
		bottomP.m_Distance = Vec3::DotProduct(bottomNormal, m_Position - GetForward() * 32.f);

		// Set the frustum
		m_Frustum = Frustum(leftP, rightP, topP, bottomP, nearP, farP);
	}

	void Camera::MarkDirty()
	{
		std::fill(m_Dirty.begin(), m_Dirty.end(), true);
	}

	void Camera::Free(Renderer* renderer)
	{
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			if (m_CameraBuffer[i].m_Buffer)
			{
				DestroyGPUBuffer(renderer->m_Allocator, m_CameraBuffer[i]);
			}
			if (m_CameraBufferStaging[i].m_Buffer)
			{
				DestroyGPUBuffer(renderer->m_Allocator, m_CameraBufferStaging[i]);
			}
		}
	}
}