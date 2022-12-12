#include "window.h"

#include "../core/utils.h"
#include "../core/renderer.h"

namespace utils::graphics::vulkan::window
	{
	window::window(core::manager& manager) :
		manager_ptr {&manager},
		surface     { manager.get_instance(), get_handle() },
		swapchain   { manager, surface.get(), client_rect.size }
		{}
	
	vk::Extent3D window::get_extent() const noexcept
		{
		utils::math::vec2u32 client_size{client_rect.size};
		return {client_size.x, client_size.y, 1 };
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
				const auto rect{client_rect};
				utils::graphics::vulkan::core::logger.log("recreate swapchain\n");
				utils::graphics::vulkan::core::logger.log("Window: "s + std::to_string(window_rect.width) + ", "s + std::to_string(window_rect.height) + "\n"s);
				utils::graphics::vulkan::core::logger.log("Client: "s + std::to_string(client_rect.width) + ", "s + std::to_string(client_rect.height) + "\n"s);

				if (client_rect.width && client_rect.height)
					{
					recreate_swapchain();
				
					if (renderer_ptr)
						{
						renderer_ptr->resize();
						
						if (resize_redraw_callback) { resize_redraw_callback(); }
						}
					}
				break;
				}
			case WM_MOVE:
				{
				utils::graphics::vulkan::core::logger.log("moving \n");
				break;
				}

			}
		return std::nullopt;
		}

	void window::recreate_swapchain() noexcept
		{
		vulkan::window::swapchain new_swapchain{*manager_ptr, surface.get(), client_rect.size, &swapchain};
		swapchain = std::move(new_swapchain);
		}
	}