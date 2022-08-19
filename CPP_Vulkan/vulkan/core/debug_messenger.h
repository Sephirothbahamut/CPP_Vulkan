#pragma once

#include <utils/memory.h>

#include <vulkan/vulkan.hpp>

#include "instance.h"

namespace utils::graphics::vulkan::core
	{
	class debug_messenger
		{
		public:
			debug_messenger(const vk::Instance& instance, const vk::DispatchLoaderDynamic& dld);

			~debug_messenger();

			inline static vk::DebugUtilsMessengerCreateInfoEXT create_info();

		private:
			vk::DebugUtilsMessengerEXT messenger;
			utils::observer_ptr<const vk::Instance> instance_ptr;
			utils::observer_ptr<const vk::DispatchLoaderDynamic> dld_ptr;

			static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
		};
	}