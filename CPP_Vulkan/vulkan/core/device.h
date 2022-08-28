#pragma once

#include "../dependencies.h"

#include "instance.h"
#include "swapchain_chosen_details.h"

namespace utils::graphics::vulkan::core
	{
	class physical_device : public vk::PhysicalDevice
		{
		public:
			physical_device(const vk::Instance& instance, /* out */ swapchain_chosen_details& swapchain_chosen_details);

			uint32_t get_graphics_family_index() const noexcept { return graphics_family_index; }
			uint32_t get_present_family_index () const noexcept { return present_family_index ; }

		private:
			uint32_t graphics_family_index;
			uint32_t present_family_index ;
		};

	struct device : vk::UniqueDevice
		{
		device(const vk::PhysicalDevice& physical_device, uint32_t graphics_family_index, uint32_t present_family_index);
		};
	}