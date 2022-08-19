#pragma once

#include <utils_win32/window.h>
#include <utils/memory.h>

#include <vulkan/vulkan.hpp>

#include "../core/manager.h"
#include "surface.h"
#include "swapchain.h"

namespace utils::graphics::vulkan::window
	{
	class window : public virtual utils::win32::window::base
		{
		public:
			window(const core::manager& manager);


			std::optional<LRESULT> procedure(UINT msg, WPARAM wparam, LPARAM lparam);

		private:
			utils::observer_ptr<const core::manager> manager_ptr;
			surface surface;
			swapchain swapchain;
		};
	}