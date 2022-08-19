#pragma once

#include <vulkan/vulkan.hpp>

namespace utils::graphics::vulkan::core
	{
	struct instance : vk::UniqueInstance
		{
		public:
			instance(const char* application_name, vk::DebugUtilsMessengerCreateInfoEXT debug_utils_messenger_cinfo);

		private:
			static vk::UniqueInstance create(const char* application_name, vk::DebugUtilsMessengerCreateInfoEXT debug_utils_messenger_cinfo) noexcept;
		};
	}