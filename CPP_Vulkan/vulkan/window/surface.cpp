#include "surface.h"

#include <vulkan/vulkan_win32.h>

namespace utils::graphics::vulkan::window
	{
	surface::surface(const vk::Instance& instance, HWND handle) :
		instance_ptr{&instance}, vk::UniqueSurfaceKHR{create(instance, handle)}
		{
		}

	vk::UniqueSurfaceKHR surface::create(const vk::Instance& instance, HWND handle)
		{
		VkWin32SurfaceCreateInfoKHR info
			{
				.sType{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR },
				.pNext{ nullptr },
				.flags{ 0 },
				.hinstance{ GetModuleHandle(nullptr) },
				.hwnd{ handle },
			};

		VkSurfaceKHR tmp_surface;

		if (vkCreateWin32SurfaceKHR(instance, &info, nullptr, &tmp_surface)) { throw std::runtime_error("Couldn't create a surface"); }
		
		vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> _deleter(instance);
		return vk::UniqueSurfaceKHR{vk::SurfaceKHR{tmp_surface}, _deleter};
		}
	}