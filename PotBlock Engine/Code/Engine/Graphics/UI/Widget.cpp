#include "Engine/Graphics/UI/Widget.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Graphics/UI/Canvas.hpp"

#include <algorithm>

VkDescriptorSetLayout PBE::Widget::s_DescriptorSetLayout = VK_NULL_HANDLE;

PBE::Widget::Widget()
{
}

PBE::Widget::~Widget()
{
	if (m_Parent)
	{
		m_Parent->RemoveChild(this);
	}
	for (auto child : m_Children)
	{
		if (child)
		{
			child->SetParent(m_Parent);
		}
	}
}

void PBE::Widget::CreateDescriptorSetLayout(VkDevice device, uint32_t maxWidgets)
{
	std::array<VkDescriptorSetLayoutBinding, 1> bindings = {};
	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[0].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (uint32_t)bindings.size();
	layoutInfo.pBindings = bindings.data();

	VkErrorCheck(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &s_DescriptorSetLayout), "Failed to create descriptor set layout");
	VkLabelObject(device, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, s_DescriptorSetLayout, "Material Descriptor Set Layout");
}

void PBE::Widget::DestroyDescriptorSetLayout(VkDevice device)
{
	if (s_DescriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(device, s_DescriptorSetLayout, nullptr);
		s_DescriptorSetLayout = VK_NULL_HANDLE;
	}
}

void PBE::Widget::MarkDirty()
{
	m_Dirty = true;
	OBB2 bounds = GetBounds();
	m_ConstantsDirty = true;
	m_Constants.m_Center = bounds.GetCenter();
	m_Constants.m_HalfExtents = bounds.GetDimensions() * 0.5f + m_CullExtentPadding;

	m_Constants.m_IBasis = bounds.GetIBasis();
	m_Constants.m_JBasis = bounds.GetJBasis();

	for (auto child : m_Children)
	{
		child->MarkDirty();
	}
}

PBE::Widget* PBE::Widget::GetHoveredWidget(Vec2 const& cursorPos)
{
	for(auto child : m_Children)
	{
		Widget* hovered = child->GetHoveredWidget(cursorPos);
		if (hovered)
		{
			return hovered;
		}
	}

	if (m_Visible && m_Enabled && m_IsHoverable && m_Parent != nullptr)
	{
		OBB2 bounds = GetBounds();
		if (bounds.IsPointInside(cursorPos))
		{
			return this;
		}
	}

	return nullptr;
}

void PBE::Widget::SetHovered(bool focused)
{
	if (m_IsHovered == focused)
	{
		return;
	}
	if (m_IsHoverable)
	{
		if (focused)
		{
			m_OnHovered.Invoke();
		}
		else
		{
			m_OnUnhovered.Invoke();
		}
	}
	m_IsHovered = focused;
	m_Dirty = true;
}

void PBE::Widget::SetFocused(bool focused)
{
	if (m_IsFocused == focused)
	{
		return;
	}
	if (focused)
	{
		m_OnFocused.Invoke();
	}
	else
	{
		m_OnUnfocused.Invoke();
	}
	m_IsFocused = focused;
	m_Dirty = true;
}

bool PBE::Widget::IsFocused() const
{
	return m_IsHovered;
}

PBE::Widget& PBE::Widget::SetPivot(Vec2 const& pivot)
{
	m_PivotOffset = pivot;
	return *this;
}

PBE::Vec2 PBE::Widget::GetPivot() const
{
	return m_PivotOffset;
}

void PBE::Widget::SetTheme(Theme* theme)
{
	if (theme == nullptr)
	{
		if (m_Parent)
		{
			m_pTheme = m_Parent->m_pTheme;
		}
		else if (m_Canvas)
		{
			m_pTheme = m_Canvas->m_pTheme;
		}
		else
		{
			m_pTheme = nullptr;
		}
	}
	else
	{
		m_pTheme = theme;
	}

	for (auto child : m_Children)
	{
		if (child)
		{
			child->SetTheme(m_pTheme);
		}
	}
}

void PBE::Widget::SetParent(Widget* parent)
{
	if(m_Parent && parent != m_Parent)
	{
		m_Parent->RemoveChild(this);
	}

	m_Parent = parent;
}

PBE::Widget* PBE::Widget::GetParent() const
{
	return m_Parent;
}

void PBE::Widget::AddChild(Widget* child)
{
	child->SetCanvas(m_Canvas);
	child->SetParent(this);
	child->SetTheme(m_pTheme);
	auto it = std::find(m_Children.begin(), m_Children.end(), child);
	if (it != m_Children.end())
	{
		return;
	}

	auto nIt = std::find(m_Children.begin(), m_Children.end(), nullptr);
	if (nIt != m_Children.end())
	{
		*nIt = child;
	}
	else
	{
		m_Children.push_back(child);
	}

	//Slower, but keeps the transform hierarchy up to date.
	UpdateTransform();
}

void PBE::Widget::RemoveChild(Widget* child)
{
	child->m_Parent = nullptr;
	child->SetCanvas(nullptr);
	child->SetTheme(nullptr);
	auto it = std::find(m_Children.begin(), m_Children.end(), child);
	if (it != m_Children.end())
	{
		*it = nullptr;
	}
}

PBE::Widget& PBE::Widget::SetVisible(bool visible, bool updateChildren)
{
	m_Visible = visible;
	if (updateChildren)
	{
		for (auto child : m_Children)
		{
			child->SetVisible(visible, updateChildren);
		}
	}
	return *this;
}

bool PBE::Widget::IsVisible() const
{
	return m_Visible;
}

PBE::Widget& PBE::Widget::SetEnabled(bool enabled, bool updateChildren)
{
	m_Enabled = enabled;
	if (updateChildren)
	{
		for (auto child : m_Children)
		{
			child->SetEnabled(enabled, updateChildren);
		}
	}
	return *this;
}

bool PBE::Widget::IsEnabled() const
{
	return m_Enabled;
}

void PBE::Widget::Update()
{
	m_OnUpdate.Invoke();
	for (auto child : m_Children)
	{
		child->Update();
	}
}

void PBE::Widget::CmdTransfer(Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
	if (!m_InitializedDescriptorSet && m_Canvas)
	{
		std::array<VkDescriptorSetLayout, MAX_FRAMES_IN_FLIGHT> layouts;
		std::fill(layouts.begin(), layouts.end(), s_DescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = renderer->GetDescriptorPool();
		allocInfo.descriptorSetCount = (uint32_t)m_DescriptorSetsInFlight.size();
		allocInfo.pSetLayouts = layouts.data();

		vkAllocateDescriptorSets(renderer->m_Device, &allocInfo, m_DescriptorSetsInFlight.data());

		m_InitializedDescriptorSet = true;
	}


	if (m_ConstantsDirty && m_Canvas)
	{
		m_ConstantsDirty = false;
		std::fill(m_DirtyConstantSwaps.begin(), m_DirtyConstantSwaps.end(), true);
		if (m_ConstantBuffer.has_value())
		{
			m_OldConstantBuffers.push_back(*m_ConstantBuffer);
		}

		m_ConstantBuffer = renderer->CmdCreateVertexBuffer(buffer, frameIndex, &m_Constants, sizeof(WidgetConstants), "Widget Constant Buffer");
	}

	if (m_DirtyConstantSwaps[frameIndex])
	{
		m_DirtyConstantSwaps[frameIndex] = false;
		if (m_ConstantBuffer.has_value())
		{
			m_ConstantBuffersInFlight[frameIndex] = m_ConstantBuffer;

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = m_ConstantBuffer->m_Buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(WidgetConstants);

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSetsInFlight[frameIndex];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(renderer->m_Device, 1, &descriptorWrite, 0, nullptr);
		}
		else
		{
			m_ConstantBuffersInFlight[frameIndex].reset();
		}
	}

	for (auto itr = m_OldConstantBuffers.begin(); itr != m_OldConstantBuffers.end();)
	{
		auto otherItr = std::find_if(
			m_ConstantBuffersInFlight.begin(), 
			m_ConstantBuffersInFlight.end(), 
			[&itr](std::optional<GPUBuffer> const& buffer) 
			{
				if (buffer.has_value())
				{
					return buffer->m_Buffer == itr->m_Buffer;
				}
				return false;
			}
		);
		if (otherItr == m_ConstantBuffersInFlight.end())
		{
			DestroyGPUBuffer(renderer->m_Allocator,*itr);
			itr = m_OldConstantBuffers.erase(itr);
		}
		else
		{
			++itr;
		}
	}

	for (auto child : m_Children)
	{
		child->CmdTransfer(renderer, buffer, frameIndex);
	}
}

void PBE::Widget::CmdPostProcessDraw(VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex)
{
	if (!m_Visible)
	{
		return;
	}

	for (auto child : m_Children)
	{
		child->CmdPostProcessDraw(buffer, layout, frameIndex);
	}
}

void PBE::Widget::Paint(CPUMesh& mesh)
{
	if (!m_Visible)
	{
		return;
	}

	for (auto child : m_Children)
	{
		child->Paint(mesh);
	}
}

void PBE::Widget::Cleanup(Renderer* renderer)
{
	if (m_InitializedDescriptorSet)
	{
		vkFreeDescriptorSets(renderer->m_Device, renderer->GetDescriptorPool(), (uint32_t)m_DescriptorSetsInFlight.size(), m_DescriptorSetsInFlight.data());
		
		for (auto& buffer : m_ConstantBuffersInFlight)
		{
			if (buffer.has_value())
			{
				if (std::find(m_OldConstantBuffers.begin(), m_OldConstantBuffers.end(), *buffer) == m_OldConstantBuffers.end())
				{
					m_OldConstantBuffers.push_back(*buffer);
				}
			}
		}

		for (auto& buffer : m_OldConstantBuffers)
		{
			DestroyGPUBuffer(renderer->m_Allocator, buffer);
		}

		m_InitializedDescriptorSet = false;
	}

	for (auto child : m_Children)
	{
		if (child)
		{
			child->Cleanup(renderer);
		}
	}
}

void PBE::Widget::OnActivated()
{
	m_OnActivated.Invoke();
}

PBE::OBB2 PBE::Widget::GetBounds() const
{
	AABB2 box = GetAxisAlignedBounds();
	Vec2 center = box.GetCenter();
	Vec2 extents = box.GetDimensions();

	Vec2 right = m_GlobalRotationTransform.GetIBasis2D();

	if (m_Parent)
	{
		OBB2 parentBounds = m_Parent->GetBounds();
		Vec2 pivot = m_Parent->GetAxisAlignedBounds().GetCenter();
		Vec2 displacement = center - pivot;
		center = m_Parent->m_GlobalRotationTransform.TransformPoint(displacement) + parentBounds.m_Center;
	}

	OBB2 obb = OBB2(center, extents, right);

	return obb;
}

PBE::AABB2 PBE::Widget::GetAxisAlignedBounds() const
{
	AABB2 anchorPos = GetAnchorPositions();

	AABB2 bbox = {};
	bbox.m_Mins = anchorPos.m_Mins + m_AnchorOffsetInPixels.m_Mins;
	bbox.m_Maxs = anchorPos.m_Maxs + m_AnchorOffsetInPixels.m_Maxs;

	return bbox;
}

PBE::Mat4 PBE::Widget::GetGlobalTransform() const
{
	return m_GlobalRotationTransform;
}

PBE::Mat4 PBE::Widget::GetLocalTransform() const
{
	return m_LocalRotationTransform;
}

void PBE::Widget::SetCanvas(Canvas* canvas)
{
	m_Canvas = canvas;

	for (auto child : m_Children)
	{
		if (child)
		{
			child->SetCanvas(canvas);
		}
	}
}

PBE::AABB2 PBE::Widget::GetAnchorPositions() const
{
	if (m_Parent)
	{
		AABB2 parentBBox = m_Parent->GetAxisAlignedBounds();
		
		AABB2 anchorPos = AABB2();
		anchorPos.m_Mins = parentBBox.GetPositionAtUV(m_NormalizedAnchors.m_Mins);
		anchorPos.m_Maxs = parentBBox.GetPositionAtUV(m_NormalizedAnchors.m_Maxs);

		return anchorPos;

	}
	
	// If no parent, return a zero AABB. This will make the units for a root widget the size of the anchor offsets.
	return AABB2(Vec2(0.f,0.f),Vec2(0.f,0.f));
}

void PBE::Widget::UpdateTransform()
{
	m_LocalRotationTransform = EulerAngles(m_Rotation, 0.f, 0.f).GetAsMatrix();
	m_GlobalRotationTransform = m_Parent ? m_LocalRotationTransform.GetAppended(m_Parent->GetGlobalTransform()) : m_LocalRotationTransform;

	for (auto child : m_Children)
	{
		child->UpdateTransform();
	}
}