#include "VerticalContainerWidget.hpp"

void PBE::VerticalContainerWidget::Update()
{
	int index = m_Children.size() - 1;
	float tPercent = 1.f / m_Children.size();
	float tOffset = 0.f;
	AABB2 bounds = GetAxisAlignedBounds();
	Vec2 size = bounds.GetDimensions();

	if (m_UseHeightOverride)
	{
		if (tPercent * size.y > m_HeightOverride)
		{
			tPercent = m_HeightOverride / size.y;
			tOffset = 1.f - tPercent * (m_Children.size() - 1);
		}
	}

	float childrenHeight = GetChildrenHeight();
	float height = bounds.GetHeight();

	if (m_UseOffsetHeight)
	{
		float currentY = bounds.m_Maxs.y;
		for (Widget* child : m_Children)
		{
			float childHeight = child->m_AnchorOffsetInPixels.GetDimensions().y;
			child->m_NormalizedAnchors.m_Maxs.y = 0.f;
			child->m_NormalizedAnchors.m_Mins.y = 0.f;
			child->m_AnchorOffsetInPixels.m_Mins.y = currentY - childHeight;
			child->m_AnchorOffsetInPixels.m_Maxs.y = currentY;
		
			if (!m_UseOffsetWidth)
			{
				child->m_NormalizedAnchors.m_Mins.x = 0.f;
				child->m_NormalizedAnchors.m_Maxs.x = 1.f;
				child->m_AnchorOffsetInPixels.m_Mins.x = 0.f;
				child->m_AnchorOffsetInPixels.m_Maxs.x = 0.f;
			}
			else
			{
				child->m_NormalizedAnchors.m_Mins.x = 0.f;
				child->m_NormalizedAnchors.m_Maxs.x = 0.f;
			}
			currentY -= childHeight;
		}
	}
	else
	{
		for (Widget* child : m_Children)
		{
			child->m_NormalizedAnchors.m_Mins.y = index * tPercent + tOffset;
			child->m_NormalizedAnchors.m_Maxs.y = (index + 1) * tPercent + tOffset;
			if (!m_UseOffsetWidth)
			{
				child->m_NormalizedAnchors.m_Mins.x = 0.f;
				child->m_NormalizedAnchors.m_Maxs.x = 1.f;
				child->m_AnchorOffsetInPixels.m_Mins.x = 0.f;
				child->m_AnchorOffsetInPixels.m_Maxs.x = 0.f;
			}
			else
			{
				child->m_NormalizedAnchors.m_Mins.x = 0.f;
				child->m_NormalizedAnchors.m_Maxs.x = 0.f;
			}
			--index;
		}
	}
	Widget::Update();
}

float PBE::VerticalContainerWidget::GetChildrenHeight() const
{
	float totalHeight = 0.f;
	for (Widget* child : m_Children)
	{
		totalHeight += child->m_AnchorOffsetInPixels.GetHeight();
	}
	return totalHeight;
}
