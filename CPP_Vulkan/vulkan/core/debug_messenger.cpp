#include "debug_messenger.h"

#include <utils/logger.h>

#include "utils.h"

namespace utils::graphics::vulkan::core
	{
	debug_messenger::debug_messenger(const vk::Instance& instance, const vk::DispatchLoaderDynamic& dld) : instance_ptr{&instance}, dld_ptr{&dld}
		{
		messenger = instance.createDebugUtilsMessengerEXT(create_info(), nullptr, dld);
		}

	debug_messenger::~debug_messenger()
		{
		instance_ptr->destroyDebugUtilsMessengerEXT(messenger, nullptr, *dld_ptr);
		}

	vk::DebugUtilsMessengerCreateInfoEXT debug_messenger::create_info()
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

	VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT , const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* )
		{
		auto severity = vk::DebugUtilsMessageSeverityFlagBitsEXT(messageSeverity);
		switch (severity)
			{
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:   
				logger.err(pCallbackData->pMessage);
				throw error{pCallbackData->pMessage}; break;
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning: logger.wrn(pCallbackData->pMessage); break;
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:    logger.inf(pCallbackData->pMessage); break;
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose: logger.log(pCallbackData->pMessage); break;
			}

		return VK_FALSE;
		}
		
	}