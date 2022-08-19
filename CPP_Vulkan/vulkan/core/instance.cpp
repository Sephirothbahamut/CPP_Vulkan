#include "instance.h"

#include <utils/cout_containers.h>
#include <utils/memory.h>

#include <Windows.h>
#include <vulkan/vulkan_win32.h>

#include "utils.h"

#include <vector>

namespace utils::graphics::vulkan::core
	{
	namespace details
		{
		struct instance_extensions
			{
			inline static const std::vector<const char*> list
				{
				VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
				VK_KHR_SURFACE_EXTENSION_NAME,
				VK_KHR_WIN32_SURFACE_EXTENSION_NAME
				};

			[[nodiscard]] static std::vector<const char*> check_missing()
				{
				static auto available{ vk::enumerateInstanceExtensionProperties() }; // static variable construction only happens once
				return check_required_in_available(list, available, [](const auto& required, const auto& available) { return strcmp(required, available.extensionName) == 0; });
				}
			};
		}
		
	instance::instance(const char* application_name, vk::DebugUtilsMessengerCreateInfoEXT debug_utils_messenger_cinfo) : vk::UniqueInstance{ create(application_name, debug_utils_messenger_cinfo) }{}

		
	vk::UniqueInstance instance::create(const char* application_name, vk::DebugUtilsMessengerCreateInfoEXT debug_utils_messenger_cinfo) noexcept
		{
		auto missing_validation_layers  {details::validation_layers  ::check_missing()};
		auto missing_instance_extensions{details::instance_extensions::check_missing()};

		bool validation_layers_incomplete  {!missing_validation_layers  .empty()};
		bool instance_extensions_incomplete{!missing_instance_extensions.empty()};
		if (validation_layers_incomplete || instance_extensions_incomplete)
			{
			using namespace utils::cout;
			std::stringstream message{"Instance creation failed "};
			if (validation_layers_incomplete)
				{
				message << "missing the following validation layers:";
				message << missing_validation_layers;
				}
			if (validation_layers_incomplete && instance_extensions_incomplete) { message << ", "; }
			if (instance_extensions_incomplete)
				{
				message << "missing the following device extensions:\n";
				message << missing_instance_extensions;
				}
			throw error{message.str()};
			}

		vk::ApplicationInfo app_info{
			.pApplicationName   {application_name},
			.applicationVersion {VK_MAKE_VERSION(1, 0, 0)},
			.pEngineName        {"IIGE"},
			.engineVersion      {VK_MAKE_VERSION(1, 0, 0)},
			.apiVersion         {VK_API_VERSION_1_1}
			};

		auto& extensions{details::instance_extensions::list};

		try {
			return vk::createInstanceUnique(
				vk::InstanceCreateInfo{
					.pNext{&debug_utils_messenger_cinfo},
					.flags{vk::InstanceCreateFlags()},
					.pApplicationInfo{&app_info},
					.enabledLayerCount{static_cast<uint32_t>(details::validation_layers::enabled ? details::validation_layers::list.size() : 0)},
					.ppEnabledLayerNames{details::validation_layers::enabled ? details::validation_layers::list.data() : nullptr},
					.enabledExtensionCount{static_cast<uint32_t>(extensions.size())},
					.ppEnabledExtensionNames{extensions.data()}
				}, nullptr);
			}
		catch (vk::SystemError err) {
			throw std::runtime_error("failed to create instance!");
			}
		}
		
	}