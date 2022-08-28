#pragma once

#include <utils_win32/window.h>
#include <utils/memory.h>
#include <utils/containers/handled_container.h>

#include "../dependencies.h"

#include "../core/manager.h"
#include "surface.h"
#include "swapchain.h"
#include "window_sized_image.h"

namespace utils::graphics::vulkan::window
	{
	class window : public virtual utils::win32::window::base
		{
		public:
			window_sized_images images;

			window(const core::manager& manager);

			std::optional<LRESULT> procedure(UINT msg, WPARAM wparam, LPARAM lparam);

			vk::Extent3D get_extent() const noexcept;

			__declspec(property(get = get_extent)) vk::Extent3D extent;

		private:
			utils::observer_ptr<const core::manager> manager_ptr;
			surface surface;
			swapchain swapchain;
		};
	}