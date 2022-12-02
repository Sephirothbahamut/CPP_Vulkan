#pragma once
#include "swapchain.h"

#include <limits>

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

namespace utils::graphics::vulkan::window
	{
	swapchain::swapchain(const core::manager& manager, const vk::SurfaceKHR& surface, utils::math::vec2u window_size, utils::observer_ptr<swapchain> old_swapchain) :
		vk_swapchain{ create_swapchain(manager, surface, window_size, (old_swapchain ? old_swapchain->vk_swapchain.get() : nullptr)) },
		images{ manager.get_device().getSwapchainImagesKHR(vk_swapchain.get()) },
		image_views{ create_image_views(manager, images) }
		{}

	const vk::ImageView& swapchain::get_image_view(size_t index) const noexcept
		{
		return image_views[index].get();
		}

	size_t swapchain::size() const noexcept
		{
		return images.size();
		}

	uint32_t swapchain::next_image(const core::manager & manager, vk::Semaphore vk_semaphore_image_available) const noexcept
		{
		auto ret{ manager.get_device()
			.acquireNextImageKHR(vk_swapchain.get(), std::numeric_limits<uint64_t>::max(), vk_semaphore_image_available, nullptr) };
		if (ret.result != vk::Result::eSuccess)
			{
			throw std::runtime_error("Failed to acquire swapchain image!");
			}
		return ret.value;
		}

	void swapchain::present(const core::manager& manager, vk::Semaphore vk_semaphore_render_finished, uint32_t image_index) const noexcept
		{
		vk::SwapchainKHR swap_chains[] = { vk_swapchain.get() };
		vk::Semaphore    signal_semaphores[] = { vk_semaphore_render_finished };
		vk::PresentInfoKHR present_info
			{
					.waitSemaphoreCount { 1 },
					.pWaitSemaphores    { signal_semaphores },
					.swapchainCount     { 1 },
					.pSwapchains        { swap_chains },
					.pImageIndices      { &image_index },
					.pResults           { nullptr },
			};
		manager.get_queues().get_present().queue.presentKHR(&present_info);
		}

	vk::UniqueSwapchainKHR swapchain::create_swapchain(const core::manager& manager, const vk::SurfaceKHR& surface, utils::math::vec2u window_size, vk::SwapchainKHR old_swapchain)
		{
		auto capabilities{ manager.get_physical_device().getSurfaceCapabilitiesKHR(surface) };
		
		auto swapchain_chosen_details{ manager.get_swapchain_chosen_details() };

		//TODO qui si fa la finestra trasparente :)
		auto composite_alpha{vk::CompositeAlphaFlagBitsKHR::eOpaque};
		if (capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
			{
			composite_alpha = vk::CompositeAlphaFlagBitsKHR::ePostMultiplied;
			}
		else if (capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
			{
			composite_alpha = vk::CompositeAlphaFlagBitsKHR::ePreMultiplied;
			}
		else if (capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
			{
			composite_alpha = vk::CompositeAlphaFlagBitsKHR::eInherit;
			}

		vk::SwapchainCreateInfoKHR info
			{
			.flags{vk::SwapchainCreateFlagsKHR()},
			.surface{surface},
			.minImageCount{swapchain_chosen_details.get_image_count()},
			.imageFormat{swapchain_chosen_details.get_format().format},
			.imageColorSpace{swapchain_chosen_details.get_format().colorSpace},
			.imageExtent{select_extent(capabilities, window_size)},
			.imageArrayLayers{1},
			.imageUsage{vk::ImageUsageFlagBits::eColorAttachment},

			.preTransform = capabilities.currentTransform,

			.compositeAlpha = composite_alpha,

			.presentMode = swapchain_chosen_details.get_present_mode(),
			.clipped = VK_TRUE,

			.oldSwapchain = old_swapchain,
			};

		if (true)
			{
			auto queues{ manager.get_queues() };
			std::array<uint32_t, 2> indices{ queues.get_graphics().index, queues.get_present().index };
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

		return manager.get_device().createSwapchainKHRUnique(info);
		}

	std::vector<vk::UniqueImageView> swapchain::create_image_views(const core::manager& manager, const std::vector<vk::Image>& swap_chain_images) const noexcept
		{
		std::vector<vk::UniqueImageView> ret(swap_chain_images.size());

		for (size_t i = 0; i < ret.size(); i++)
			{
			vk::ImageViewCreateInfo create_info
				{
					.image = swap_chain_images[i],
					.viewType = vk::ImageViewType::e2D,
					.format = manager.get_swapchain_chosen_details().get_format().format,
					.components
					{
						.r = vk::ComponentSwizzle::eIdentity,
						.g = vk::ComponentSwizzle::eIdentity,
						.b = vk::ComponentSwizzle::eIdentity,
						.a = vk::ComponentSwizzle::eIdentity,
					},
					.subresourceRange
					{
						.aspectMask = vk::ImageAspectFlagBits::eColor,
						.baseMipLevel = 0,
						.levelCount = 1,
						.baseArrayLayer = 0,
						.layerCount = 1,
					}
				};
			ret[i] = manager.get_device().createImageViewUnique(create_info);
			}

		return ret;
		}
	}