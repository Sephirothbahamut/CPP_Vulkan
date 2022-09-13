#pragma once

#include <utils_win32/window.h>
#include <utils/memory.h>
#include <utils/containers/handled_container.h>

#include "../dependencies.h"

#include "../core/manager.h"
#include "../core/renderer_window_data.h"

#include "surface.h"
#include "swapchain.h"


namespace utils::graphics::vulkan::window
	{
	class window : public virtual utils::win32::window::base
		{
		friend void core::manager::bind(vulkan::window::window& window, core::renderer& renderer);
		public:
			utils::observer_ptr<core::renderer> renderer_ptr = nullptr;
			window(core::manager& manager);
			~window();

			std::optional<LRESULT> procedure(UINT msg, WPARAM wparam, LPARAM lparam);

			vk::Extent3D get_extent() const noexcept;

			const swapchain& get_swapchain() const noexcept;

			__declspec(property(get = get_extent)) vk::Extent3D extent;

			class getter_renderer_window_data
				{
				friend class core::renderer_window_data;
				friend class window;

				getter_renderer_window_data(window& window);

				std::vector<observer_ptr<core::renderer_window_data>>& renderer_dependent_data_ptrs() noexcept;

				utils::observer_ptr<window> window_ptr;
				};

			friend class getter_renderer_window_data;
			inline getter_renderer_window_data getter(core::renderer_window_data*) noexcept { return { *this }; }
		private:
			utils::observer_ptr<core::manager> manager_ptr;
			surface surface;
			swapchain swapchain;
			std::vector<observer_ptr<core::renderer_window_data>> renderer_dependent_data_ptrs;
		};
	}