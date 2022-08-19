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

				surface = {core::manager::getter_window{*manager_ptr}.instance(), get_handle()};
				swapchain = {*manager_ptr, surface.get(), size};
				}
			}
		return std::nullopt;
		}
	}