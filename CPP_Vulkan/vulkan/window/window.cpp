#include "window.h"

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

	std::optional<LRESULT> window::procedure(UINT msg, WPARAM w, LPARAM l)
		{
		switch (msg)
			{
				case WM_SIZE:
				{
				std::cout << "recreate swapchain\n";
				std::cout << width << ", " << height << "\n";

				if (width && height)
					{
					vulkan::window::swapchain new_swapchain{ *manager_ptr, surface.get(), size, &swapchain };
					swapchain = std::move(new_swapchain);

					images.update_images({ width, height, 1 });

					for (auto renderer_ptr : renderer_ptrs)
						{
						auto& renderer{ *renderer_ptr };
						renderer.resize(*manager_ptr, *this);

						renderer.draw(*manager_ptr, *this);
						}
					}
				}
				case WM_MOVE:
				{
					
				if (width && height)
					{
					for (auto renderer_ptr : renderer_ptrs)
						{
						auto& renderer{ *renderer_ptr };

						renderer.draw(*manager_ptr, *this);
						}
					}
				}
				case WM_WINDOWPOSCHANGING: return DefWindowProcW(get_handle(), msg, w, l);
				//TODO use this instead of size/move hoping it vorks :)
				

				//case WM_MOUSEHOVER: case WM_MOUSEMOVE: case WM_MOUSELEAVE: case WM_NCMOUSELEAVE: case WM_NCHITTEST: case WM_SETCURSOR: case WM_NCMOUSEMOVE: case WM_GETICON: 
				//case WM_WINDOWPOSCHANGED: case WM_WINDOWPOSCHANGING:
				//case WM_NCLBUTTONDOWN: case WM_NCLBUTTONUP: case WM_SYSCOMMAND: 
				//case WM_CAPTURECHANGED: case WM_MOVING: case WM_GETMINMAXINFO:
				//case WM_NCCALCSIZE:
				//case WM_ENTERSIZEMOVE:
				//case WM_EXITSIZEMOVE:
				//break;
				//
				//default: 
				//	if(true)
				//	{
				//		std::cout << "pippo" << std::endl;
				//	}

			}
		return std::nullopt;
		}
	}