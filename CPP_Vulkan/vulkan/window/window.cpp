#include "window.h"

#include "../core/utils.h"

namespace utils::graphics::vulkan::window
	{
	window::window(core::manager& manager, std::vector<utils::observer_ptr<core::renderer>>& renderer_ptrs) :
		manager_ptr{&manager},
		renderer_ptrs{renderer_ptrs},
		surface    { manager.getter(this).instance(), get_handle() },
		swapchain  { manager, surface.get(), size },
		images     { manager, {width, height, 1} }
		{}
	
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
				
					images.update_images({ width, height, 1 });
				
					for (auto renderer_ptr : renderer_ptrs)
						{
						auto& renderer{ *renderer_ptr };
						renderer.resize(*manager_ptr, *this);
				
						renderer.draw(*manager_ptr, *this, 0);
						}
					}
				break;
				}
				case WM_MOVE:
				{
				utils::graphics::vulkan::core::logger.log("moving \n");
				if (width && height)
					{
					for (auto renderer_ptr : renderer_ptrs)
						{
						auto& renderer{ *renderer_ptr };
				
						//renderer.draw(*manager_ptr, *this); TODO
						}
					}
				break;
				}

			}
		return std::nullopt;
		}
	}