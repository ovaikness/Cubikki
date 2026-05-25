#pragma once
#include "Engine/Graphics/UI/Widget.hpp"
#include "Engine/Graphics/UI/Canvas.hpp"

namespace PBE
{
	class VerticalContainerWidget : public Widget
	{
	public:
		bool  m_UseOffsetHeight{ false };
		bool  m_UseOffsetWidth{ false };
		bool  m_UseHeightOverride{ false };
		float m_HeightOverride{ 0.f };

		Vec2 m_Alignment{ 0.f, 0.f };
	public:
		virtual void Update() override;
		float GetChildrenHeight() const;
	};
}