#include "window.h"

namespace utils::graphics::vulkan::window
	{
	
	window::window(const core::manager& manager) :
		manager_ptr{ &manager },
		surface{ core::manager::getter_window{manager}.instance(), get_handle()},
		swapchain{ manager, surface.get(), size}
		{}

	std::optional<LRESULT> window::procedure(UINT msg, WPARAM wparam, LPARAM lparam)
		{
		switch (msg)
			{
				case WM_SIZE:
				{
				std::cout << "recreate swapchain\n";
				std::cout << width << ", " << height << "\n";
				}
			}
		return std::nullopt;
		}
	}