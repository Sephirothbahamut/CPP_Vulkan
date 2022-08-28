#pragma once
#include "swapchain.h"

namespace utils::graphics::vulkan::window
	{
	namespace details
		{
		vk::Extent2D select_extent(vk::SurfaceCapabilitiesKHR capabilities, utils::math::vec2u window_size)
			{
			return
				{
				capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max() ? capabilities.currentExtent :
				  vk::Extent2D
					{
					std::clamp(window_size.x, capabilities.minImageExtent.width , capabilities.maxImageExtent.width),
					std::clamp(window_size.y, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
					}
				};
			}
		}
		swapchain::swapchain(const core::manager& manager, const vk::SurfaceKHR& surface, utils::math::vec2u window_size, vk::SwapchainKHR old_swapchain) :
			vk::UniqueSwapchainKHR{create(manager, surface, window_size, old_swapchain)}
			{}

		vk::UniqueSwapchainKHR swapchain::create(const core::manager& manager, const vk::SurfaceKHR& surface, utils::math::vec2u window_size, vk::SwapchainKHR old_swapchain)
			{
			auto capabilities{ manager.getter(this).physical_device().getSurfaceCapabilitiesKHR(surface)};
			auto swapchain_chosen_details{ manager.getter(this).swapchain_chosen_details() };

			vk::SwapchainCreateInfoKHR info
				{
				.flags{vk::SwapchainCreateFlagsKHR()},
				.surface{surface},
				.minImageCount{swapchain_chosen_details.get_image_count()},
				.imageFormat{swapchain_chosen_details.get_format().format},
				.imageColorSpace{swapchain_chosen_details.get_format().colorSpace},
				.imageExtent{details::select_extent(capabilities, window_size)},
				.imageArrayLayers{1},
				.imageUsage{vk::ImageUsageFlagBits::eColorAttachment},

				.preTransform = capabilities.currentTransform,

				.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque, //TODO qui si fa la finestra trasparente :)

				.presentMode = swapchain_chosen_details.get_present_mode(),
				.clipped = VK_TRUE,

				.oldSwapchain = old_swapchain,
				};

			if (true)
				{
				auto queues{ manager.getter(this).queues() };
				std::array<uint32_t, 2> indices{ queues.get_graphics().index, queues.get_present().index};
				if (queues.get_graphics().index == queues.get_present().index)
					{
					info.imageSharingMode = vk::SharingMode::eExclusive;
					info.queueFamilyIndexCount = 2;
					info.pQueueFamilyIndices = indices.data();
					}
				else
					{
					info.imageSharingMode = vk::SharingMode::eConcurrent;
					info.queueFamilyIndexCount = 0; // Optional
					info.pQueueFamilyIndices = nullptr; // Optional
					}
				}

			return manager.getter(this).device().createSwapchainKHRUnique(info);
			}
	}