#pragma once

#include <utils/memory.h>

#include <Windows.h>
#include <vulkan/vulkan.hpp>

namespace utils::graphics::vulkan::window
	{
	class unique_surface
		{
		public:
			unique_surface(const vk::Instance& instance, HWND handle);

			unique_surface(const unique_surface& other) = delete;
			unique_surface& operator=(const unique_surface& other) = delete;

			const vk::SurfaceKHR& get() const noexcept;

			~unique_surface();
		private:
			utils::observer_ptr<const vk::Instance> instance_ptr;
			vk::SurfaceKHR surface{ nullptr };
		};
	}