#pragma once

#include <optional>

#include <utils/memory.h>
#include <utils/math/vec2.h>

#include "../dependencies.h"

#include "../core/manager.h"

namespace utils::graphics::vulkan::window
	{
	class swapchain
		{
		public:
			swapchain(const core::manager& manager, const vk::SurfaceKHR& surface, utils::math::vec2u window_size, utils::observer_ptr<swapchain> old_swapchain = nullptr);

			const vk::ImageView& get_image_view(size_t index) const noexcept;
			uint32_t next_image(const core::manager& manager, vk::Semaphore vk_semaphore_image_available) const noexcept;

			//TODO temporary, to remove from here for further generalization: multiple windows, multiple swapchains and present is called with all of them
			void present(const core::manager& manager, vk::Semaphore vk_semaphore_render_finished, uint32_t image_index) const noexcept;

			size_t size() const noexcept;
		private:
			vk::UniqueSwapchainKHR vk_swapchain;
			std::vector<vk::Image> images; // not unique since the owner is vk::SwapchainKHR
			std::vector<vk::UniqueImageView> image_views;

			vk::UniqueSwapchainKHR create_swapchain(const core::manager& manager, const vk::SurfaceKHR& surface, utils::math::vec2u window_size, vk::SwapchainKHR old_swapchain = nullptr);
			std::vector<vk::UniqueImageView> create_image_views(const core::manager& manager, const std::vector<vk::Image>& swap_chain_images) const noexcept;
		};
	}