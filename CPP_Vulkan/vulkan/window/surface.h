#pragma once

#include <utils/memory.h>

#include <Windows.h>
#include <vulkan/vulkan.hpp>

namespace utils::graphics::vulkan::window
	{
	class surface : public vk::UniqueSurfaceKHR
		{
		public:
			surface(const vk::Instance& instance, HWND handle);

		private:
			utils::observer_ptr<const vk::Instance> instance_ptr;
			
			vk::UniqueSurfaceKHR create(const vk::Instance& instance, HWND handle);
		};
	}