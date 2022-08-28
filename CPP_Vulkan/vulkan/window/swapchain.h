#pragma once

#include <utils/memory.h>
#include <utils/math/vec2.h>

#include "../dependencies.h"

#include "../core/manager.h"

namespace utils::graphics::vulkan::window
	{
	class swapchain : public vk::UniqueSwapchainKHR
		{
		public:
			swapchain(const core::manager& manager, const vk::SurfaceKHR& surface, utils::math::vec2u window_size, vk::SwapchainKHR old_swapchain = nullptr);

		private:
			vk::UniqueSwapchainKHR create(const core::manager& manager, const vk::SurfaceKHR& surface, utils::math::vec2u window_size, vk::SwapchainKHR old_swapchain = nullptr);
		};
	}