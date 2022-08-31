#include "window.h"

namespace utils::graphics::vulkan::window
	{
	
	window::window(const core::manager& manager) :
		manager_ptr{ &manager },
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

	std::optional<LRESULT> window::procedure(UINT msg, WPARAM , LPARAM )
		{
		switch (msg)
			{
				case WM_SIZE:
				{
				std::cout << "recreate swapchain\n";
				std::cout << width << ", " << height << "\n";
				
				vulkan::window::swapchain new_swapchain { *manager_ptr, surface.get(), size, &swapchain };
				swapchain = std::move(new_swapchain);

				images.update_images({width, height, 1});
				}
			}
		return std::nullopt;
		}
	}