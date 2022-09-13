#include "window.h"

#include "../core/utils.h"
#include "../core/renderer.h"

namespace utils::graphics::vulkan::window
	{
	window::window(core::manager& manager) :
		manager_ptr{&manager},
		surface    { manager.getter(this).instance(), get_handle() },
		swapchain  { manager, surface.get(), size }
		{}
	window::~window()
		{
		for (auto renderer_dependent_data_ptr : renderer_dependent_data_ptrs)
			{
			delete renderer_dependent_data_ptr;
			}
		}
	
	vk::Extent3D window::get_extent() const noexcept
		{
		return { width, height, 1 };
		}

	const swapchain& window::get_swapchain() const noexcept
		{
		return swapchain;
		}

	// see this :) https://wiki.winehq.org/List_Of_Windows_Messages
	std::optional<LRESULT> window::procedure(UINT msg, WPARAM w, LPARAM l)
		{
		using namespace std::string_literals;
		switch (msg)
			{
				case WM_SIZE:
				{
				utils::graphics::vulkan::core::logger.log("recreate swapchain\n");
				utils::graphics::vulkan::core::logger.log(std::to_string(width) + ", "s + std::to_string(height) + "\n"s);
				if (width && height)
					{
					vulkan::window::swapchain new_swapchain{ *manager_ptr, surface.get(), size, &swapchain };
					swapchain = std::move(new_swapchain);
				
					for (auto& data : renderer_dependent_data_ptrs)
						{
						
						data->getter(this).resize(*manager_ptr, *this);
						
						if (renderer_ptr) 
							{
							renderer_ptr->draw(*manager_ptr, *this, 0);
							}
						}
					}
				break;
				}
				case WM_MOVE:
				{
				utils::graphics::vulkan::core::logger.log("moving \n");
				if (width && height)
					{
					}
				break;
				}

			}
		return std::nullopt;
		}

	// getter_renderer_window_data

	window::getter_renderer_window_data::getter_renderer_window_data(window& window) : window_ptr{ &window } {}

	std::vector<observer_ptr<core::renderer_window_data>>& window::getter_renderer_window_data::renderer_dependent_data_ptrs() noexcept
		{
		return window_ptr->renderer_dependent_data_ptrs;
		}
	}