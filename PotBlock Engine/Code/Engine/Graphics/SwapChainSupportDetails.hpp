#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace PBE
{
	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR m_Capabilities{};
		std::vector<VkSurfaceFormatKHR> m_Formats;
		std::vector<VkPresentModeKHR> m_PresentModes;
	};

}