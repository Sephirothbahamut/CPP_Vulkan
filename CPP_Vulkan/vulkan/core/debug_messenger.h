#pragma once

#include <utils/memory.h>
#include <utils/logger.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_win32.h>

#include "utils.h"
#include "instance.h"

namespace utils::graphics::vulkan
	{
	class debug_messenger
		{
		public:
			debug_messenger(const vk::Instance& instance, const vk::DispatchLoaderDynamic& dld) : instance_ptr{&instance}, dld_ptr{&dld}
				{
				if (!details::validation_layers::enabled) return;

				messenger = instance.createDebugUtilsMessengerEXT(create_info(), nullptr, dld);
				}

			~debug_messenger()
				{
				instance_ptr->destroyDebugUtilsMessengerEXT(messenger, nullptr, *dld_ptr);
				}

			inline static vk::DebugUtilsMessengerCreateInfoEXT create_info()
				{
				constexpr vk::DebugUtilsMessageSeverityFlagsEXT full   {vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError};
				constexpr vk::DebugUtilsMessageSeverityFlagsEXT limited{vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError};

				return {
					.flags{vk::DebugUtilsMessengerCreateFlagsEXT()},
					.messageSeverity{limited},
					.messageType{vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance},
					.pfnUserCallback{debug_callback}
					};
				}

		private:
			vk::DebugUtilsMessengerEXT messenger;
			utils::observer_ptr<const vk::Instance> instance_ptr;
			utils::observer_ptr<const vk::DispatchLoaderDynamic> dld_ptr;

			static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
				{
				auto severity = vk::DebugUtilsMessageSeverityFlagBitsEXT(messageSeverity);
				switch (severity)
					{
					case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:   logger.err(pCallbackData->pMessage); throw error{pCallbackData->pMessage}; break;
					case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning: logger.wrn(pCallbackData->pMessage); break;
					case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:    logger.inf(pCallbackData->pMessage); break;
					case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose: logger.log(pCallbackData->pMessage); break;
					}

				return VK_FALSE;
				}
		};
	}