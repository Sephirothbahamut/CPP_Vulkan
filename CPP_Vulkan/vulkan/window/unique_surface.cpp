#include "unique_surface.h"

#include <vulkan/vulkan_win32.h>

namespace utils::graphics::vulkan::window
	{
	unique_surface::unique_surface(const vk::Instance& instance, HWND handle) :
		instance_ptr{ &instance }
		{
		VkWin32SurfaceCreateInfoKHR info
			{
				.sType{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR },
				.pNext{ nullptr },
				.flags{ 0 },
				.hinstance{ GetModuleHandle(nullptr) },
				.hwnd{ handle },
			};

		VkSurfaceKHR temp_surface;
		if (vkCreateWin32SurfaceKHR(instance, &info, nullptr, &temp_surface)) { throw std::runtime_error("Couldn't create a surface"); }
		surface = temp_surface;
		}

	const vk::SurfaceKHR& unique_surface::get() const noexcept
		{
		return surface;
		}

	unique_surface::~unique_surface()
		{
		if (surface) { instance_ptr->destroySurfaceKHR(surface); }
		}
	}