#pragma once

#include <utils_win32/window.h>
#include <utils/memory.h>
#include <utils/containers/handled_container.h>

#include "../dependencies.h"

#include "../core/manager.h"
#include "../core/renderer.h"
#include "surface.h"
#include "swapchain.h"
#include "window_sized_image.h"

namespace utils::graphics::vulkan::core { class renderer; }
namespace utils::graphics::vulkan::window
	{
	class window : public virtual utils::win32::window::base
		{
		public:
			window(core::manager& manager, std::vector<utils::observer_ptr<core::renderer>>& renderer_ptrs);

			std::optional<LRESULT> procedure(UINT msg, WPARAM wparam, LPARAM lparam);

			vk::Extent3D get_extent() const noexcept;

			const swapchain& get_swapchain() const noexcept;

			__declspec(property(get = get_extent)) vk::Extent3D extent;

			window_sized_images images;

		private:
			utils::observer_ptr<core::manager> manager_ptr;
			surface surface;
			swapchain swapchain;
			std::vector<utils::observer_ptr<core::renderer>> renderer_ptrs;
		};
	}