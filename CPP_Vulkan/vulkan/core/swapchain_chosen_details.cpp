#pragma once
#include "swapchain_chosen_details.h"

namespace utils::graphics::vulkan::core
	{
	swapchain_chosen_details::swapchain_chosen_details(vk::SurfaceCapabilitiesKHR capabilities, std::vector<vk::SurfaceFormatKHR> formats, std::vector<vk::PresentModeKHR> present_modes) :
				format{ choose_format(formats) }, present_mode{ choose_present_mode(present_modes) }, image_count{ choose_image_count(capabilities) }{}

	const vk::SurfaceFormatKHR& swapchain_chosen_details::get_format() const noexcept
		{
		return format;
		}
	const vk::PresentModeKHR& swapchain_chosen_details::get_present_mode() const noexcept
		{
		return present_mode;
		}
	uint32_t swapchain_chosen_details::get_image_count() const noexcept
		{
		return image_count;
		}

	vk::SurfaceFormatKHR swapchain_chosen_details::choose_format(const std::vector<vk::SurfaceFormatKHR>& available_formats) noexcept
		{
		for (const auto& available_format : available_formats)
			{
			if (available_format.format == vk::Format::eB8G8R8A8Unorm && available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
				{
				return available_format;
				}
			}

		return available_formats[0];
		}

	vk::PresentModeKHR swapchain_chosen_details::choose_present_mode(const std::vector<vk::PresentModeKHR> available_present_modes) noexcept
		{
		for (const auto& available_present_mode : available_present_modes)
			{
			if (available_present_mode == vk::PresentModeKHR::eMailbox) // only for nvidia, like fifo but with a queue
				{
				return available_present_mode;
				}
			if (available_present_mode == vk::PresentModeKHR::eImmediate) // uncapped present mode, expect tearing from this
				{
				return available_present_mode;
				}
			}
		return vk::PresentModeKHR::eFifo;    // FIFO mode is always guaranteed (ensures V-Sync)
		}

	uint32_t swapchain_chosen_details::choose_image_count(const vk::SurfaceCapabilitiesKHR& capabilities) noexcept
		{
		uint32_t image_count = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
			{
			image_count = capabilities.maxImageCount;
			}
		return image_count;
		}
	}