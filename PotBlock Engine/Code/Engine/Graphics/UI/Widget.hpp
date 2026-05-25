#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Mat4.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Rotor2D.hpp"
#include "Engine/Graphics/Mesh.hpp"
#include "Engine/Graphics/Renderer.hpp"
#include "Engine/Graphics/Material.hpp"
#include "Engine/Graphics/BitmapFontSystem.hpp"
#include "Engine/Graphics/Painters/NineSlice.hpp"

#include "Engine/Graphics/GPUBuffer.hpp"
#include "Engine/Graphics/UI/Theme.hpp"

#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EventDispatcher.hpp"

#include <optional>
#include <vector>

namespace PBE
{
	class Canvas;

	struct WidgetCreateInfo
	{
		std::string m_Name;

		VkDevice m_Device;
		VmaAllocator m_Allocator;
	};

	struct WidgetConstants
	{
		Vec2 m_Center;
		Vec2 m_HalfExtents;
		Vec2 m_IBasis;
		Vec2 m_JBasis;
	};

	class Widget
	{
	public:
		static VkDescriptorSetLayout s_DescriptorSetLayout;

		bool m_Dirty{ true };
		bool m_Enabled{ true };
		bool m_Visible{ true };

		bool m_IsHoverable{ false };

		bool m_IsFocused{ false };
		bool m_IsHovered{ false };

		Theme* m_pTheme = nullptr;

		bool m_ConstantsDirty{ true };
		WidgetConstants m_Constants{};
		std::optional<GPUBuffer> m_ConstantBuffer{};
		std::array<bool, MAX_FRAMES_IN_FLIGHT> m_DirtyConstantSwaps{ true };
		std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> m_DescriptorSetsInFlight{ VK_NULL_HANDLE };
		std::array<std::optional<GPUBuffer>, MAX_FRAMES_IN_FLIGHT> m_ConstantBuffersInFlight{};
		std::vector<GPUBuffer> m_OldConstantBuffers{};

		NamedProperties m_Properties{};
		Rgba8 m_Color = Rgba8(255,255,255,128);

		AABB2 m_NormalizedAnchors = AABB2::ZERO_TO_ONE;
		AABB2 m_AnchorPositions = AABB2::ZERO_TO_ONE;
		AABB2 m_AnchorOffsetInPixels = AABB2(Vec2(0, 0), Vec2(0, 0));
		Vec2  m_CullExtentPadding{ 0,0 };

		Vec2 m_MinimumSize{ 0,0 };
		Vec2 m_PivotOffset{ 0,0 };
		Vec2 m_Scale{ 1.f, 1.f };
		float m_Rotation{ 0.f };

		EventDispatcher<> m_OnActivated{};
		EventDispatcher<> m_OnHovered{};
		EventDispatcher<> m_OnUnhovered{};
		EventDispatcher<> m_OnFocused{};
		EventDispatcher<> m_OnUnfocused{};
		EventDispatcher<> m_OnUpdate{};
	public:
		Widget();
		virtual ~Widget();

		static void CreateDescriptorSetLayout(VkDevice device, uint32_t maxWidgets);
		static void DestroyDescriptorSetLayout(VkDevice device);

		virtual void MarkDirty();

		Widget* GetHoveredWidget(Vec2 const& cursorPos);

		virtual void SetHovered(bool focused);
		virtual void SetFocused(bool focused);

		virtual bool IsFocused() const;

		virtual Widget& SetPivot(Vec2 const& pivot);
		virtual Vec2 GetPivot() const;

		virtual void SetTheme(Theme* theme);

		virtual void SetParent(Widget* parent);
		virtual Widget* GetParent() const;

		virtual void AddChild(Widget* child);
		virtual void RemoveChild(Widget* child);

		virtual Widget& SetVisible(bool visible, bool updateChildren);
		virtual bool IsVisible() const;

		virtual Widget& SetEnabled(bool enabled, bool updateChildren);
		virtual bool IsEnabled() const;
		
		virtual void Update();

		virtual void CmdTransfer(Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex);
		virtual void CmdPostProcessDraw(VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex);

		virtual void Paint(CPUMesh& mesh);

		virtual void Cleanup(Renderer* renderer);

		virtual void OnActivated();

		OBB2  GetBounds() const;
		AABB2 GetAxisAlignedBounds() const;

		void UpdateTransform();
		Mat4 GetGlobalTransform() const;
		Mat4 GetLocalTransform() const;
	protected:
		//If parent == nullptr, then this is a root widget it is the canvas.

		Canvas* m_Canvas = nullptr;
		Widget* m_Parent = nullptr;
		GPUMesh* m_Mesh = nullptr;

		Mat4 m_LocalRotationTransform = Mat4::IDENTITY;
		Mat4 m_GlobalRotationTransform = Mat4::IDENTITY;

		std::vector<Widget*> m_Children{};

		bool m_InitializedDescriptorSet{ false };
	private:
		friend class Canvas;
		void  SetCanvas(Canvas* canvas);
		AABB2 GetAnchorPositions() const;
	};
}