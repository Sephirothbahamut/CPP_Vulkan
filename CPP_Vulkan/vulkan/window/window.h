#pragma once

#include <functional>

#include <utils_win32/window.h>
#include <utils/memory.h>
#include <utils/containers/handled_container.h>

#include "../dependencies.h"

#include "../core/manager.h"
#include "../core/renderer_window_data.h"

#include "surface.h"
#include "swapchain.h"

namespace utils::graphics::vulkan::core { class renderer; }

namespace utils::graphics::vulkan::window
	{
	class window : public virtual utils::win32::window::base
		{
		friend class core::renderer;
		public:

			window(core::manager& manager);

			std::optional<LRESULT> procedure(UINT msg, WPARAM wparam, LPARAM lparam);

			vk::Extent3D get_extent() const noexcept;

			const swapchain& get_swapchain() const noexcept;

			__declspec(property(get = get_extent)) vk::Extent3D extent;

			std::function<void()> resize_redraw_callback;

		private:
			utils::observer_ptr<core::manager> manager_ptr;
			utils::observer_ptr<core::renderer> renderer_ptr;
			surface surface;
			swapchain swapchain;
		};
	}