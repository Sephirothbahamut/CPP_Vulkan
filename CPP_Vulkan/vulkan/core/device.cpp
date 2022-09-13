#include "device.h"

#include <optional>
#include <string>

#include <Windows.h>
#include <vulkan/vulkan_win32.h>

#include <utils/output/std_containers.h>

#include "utils.h"
#include "instance.h"
#include "debug_messenger.h"
#include "queues.h"

namespace utils::graphics::vulkan::core
	{
	namespace details
		{
		class proxy_surface
			{
			public:
				proxy_surface(const vk::Instance& instance) : instance(&instance)
					{
					// Register the window class.
					WNDCLASS wc = { };
					wc.lpfnWndProc = DefWindowProc;//WindowProc;
					wc.hInstance = GetModuleHandle(nullptr);
					wc.lpszClassName = CLASS_NAME;

					RegisterClass(&wc);

					// Create the window.
					hwnd = CreateWindowEx(
						0,                              // Optional window styles.
						CLASS_NAME,                     // Window class
						L"Proxy window",                // Window text
						WS_OVERLAPPEDWINDOW,            // Window style

						// Size and position
						CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

						NULL,                           // Parent window    
						NULL,                           // Menu
						GetModuleHandle(nullptr),       // Instance handle
						NULL                            // Additional application data
					);

					VkWin32SurfaceCreateInfoKHR info
						{
							.sType{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR },
							.pNext{nullptr},
							.flags{0},
							.hinstance{GetModuleHandle(nullptr)},
							.hwnd{hwnd},
						};

					VkSurfaceKHR temp_surface;
					if (vkCreateWin32SurfaceKHR(instance, &info, nullptr, &temp_surface)) { throw error("Couldn't create a surface"); }

					surface = temp_surface;
					}

				const vk::SurfaceKHR& get() const
					{
					return surface;
					}

				~proxy_surface()
					{
					instance->destroySurfaceKHR(surface);
					DestroyWindow(hwnd);
					UnregisterClass(CLASS_NAME, nullptr);
					}
			private:
				vk::SurfaceKHR surface;
				HWND hwnd;
				inline static constexpr wchar_t CLASS_NAME[] = L"Proxy";
				utils::observer_ptr<const vk::Instance> instance;
			};

		struct device_extensions
			{
			inline static std::vector<const char*> get_required()
				{
				return
					{
					VK_KHR_SWAPCHAIN_EXTENSION_NAME,
					};
				}

			[[nodiscard]] static std::vector<const char*> check_missing(const vk::PhysicalDevice& physical_device)
				{
				auto available{ physical_device.enumerateDeviceExtensionProperties() };
				return check_required_in_available(get_required(), available, [](const auto& required, const auto& available) { return strcmp(required, available.extensionName) == 0; });
				}

			};

		class queue_indices
			{
			friend physical_device::physical_device(const vk::Instance& instance, swapchain_chosen_details& swapchain_chosen_details);
			public:
				queue_indices(vk::PhysicalDevice device, const proxy_surface& proxy_surface)
					{
					auto queue_families = device.getQueueFamilyProperties();

					int i = 0;
					for (const auto& queue_family : queue_families)
						{
						if (queue_family.queueCount > 0)
							{
							if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics) { graphics_family = i; }
							if (device.getSurfaceSupportKHR(i, proxy_surface.get())) { present_family = i; }
							//if (vkGetPhysicalDeviceWin32PresentationSupportKHR(device, i)) { present_family = i; } // only for win32, surfaceless
							if (is_complete()) { break; }
							}
						i++;
						}
					}

				bool is_complete() const noexcept
					{
					return graphics_family.has_value() && present_family.has_value();
					}

			private:
				std::optional<uint32_t> graphics_family;
				std::optional<uint32_t> present_family;
			};

		float evaluate_device(const vk::PhysicalDevice& physical_device, const proxy_surface& proxy_surface)
			{
			auto missing_device_extensions{device_extensions::check_missing(physical_device)};

			//TODO actually calculate the scoring fr (thru features / properties )
			bool queue_families_incomplete{!queue_indices{ physical_device, proxy_surface }.is_complete()};
			bool device_extensions_incomplete{!missing_device_extensions.empty()};
			if (queue_families_incomplete || device_extensions_incomplete)
				{
				auto device_properties{physical_device.getProperties()};
				std::stringstream message{"Device "};
				message << device_properties.deviceName << " ";
				
				if (queue_families_incomplete) { message << "queue families are incomplete"; }
				if (queue_families_incomplete && device_extensions_incomplete) { message << ", "; }
				if (device_extensions_incomplete)
					{
					using namespace utils::output;
					message << "missing the following device extensions:\n";
					message << missing_device_extensions;
					}
				utils::graphics::vulkan::core::logger.log(message.str());
				return 0.f;
				}

			// Must be checked after ensuring extensions' availability
			// Getting swap chain capabilities from proxy surface
			auto formats{ physical_device.getSurfaceFormatsKHR(proxy_surface.get()) };
			auto present_modes{ physical_device.getSurfacePresentModesKHR(proxy_surface.get()) };

			bool swap_chain_inadequate{formats.empty() || present_modes.empty()};

			if (swap_chain_inadequate)
				{
				auto device_properties{physical_device.getProperties()};
				std::stringstream message{"Device "};
				message << device_properties.deviceName << " swap chain inadequate: missing formats or present modes";
				logger.log(message.str());
				return 0.f;
				}
				
			return 1.f;
			}
		}

	physical_device::physical_device(const vk::Instance& instance, swapchain_chosen_details& swapchain_chosen_details)
		{
			auto devices = instance.enumeratePhysicalDevices();
			if (devices.empty())
				{
				throw error{ "Failed to find GPUs with Vulkan support!" };
				}

			vk::PhysicalDevice chosen;
			details::proxy_surface proxy_surface{ instance };

			float max_score{ 0.f };

			for (const auto& device : devices)
				{
				float current_score{ details::evaluate_device(device, proxy_surface) };
				if (current_score > max_score)
					{
					chosen = device;
					max_score = current_score;
					}
				}

			if (!max_score)
				{
				throw error("Failed to find a suitable GPU!");
				}

			this->vk::PhysicalDevice::operator=(chosen);

			details::queue_indices  selected_indices{chosen, proxy_surface};
			graphics_family_index = selected_indices.graphics_family.value();
			present_family_index  = selected_indices.present_family .value();

			swapchain_chosen_details =
				{
				chosen.getSurfaceCapabilitiesKHR(proxy_surface.get()),
				chosen.getSurfaceFormatsKHR     (proxy_surface.get()),
				chosen.getSurfacePresentModesKHR(proxy_surface.get())
				};

			using namespace std::string_literals;
			logger.log("Selected device: \""s + chosen.getProperties().deviceName.data() + "\"");
		}

	device::device(const vk::PhysicalDevice& physical_device, uint32_t graphics_family_index, uint32_t present_family_index)
		{
		std::vector<vk::DeviceQueueCreateInfo> queue_create_info;
		std::set<uint32_t> unique_queue_families{ graphics_family_index, present_family_index };

		float queue_priority = 1.0f;

		for (auto queue_family : unique_queue_families)
			{
			queue_create_info.push_back(vk::DeviceQueueCreateInfo{
				.flags{vk::DeviceQueueCreateFlags()},
				.queueFamilyIndex{static_cast<unsigned int>(queue_family)},
				.queueCount{1},
				.pQueuePriorities{&queue_priority}
				});
			}

		auto device_features{ vk::PhysicalDeviceFeatures() };
		auto device_extensions{ details::device_extensions::get_required() };

		try
			{
			this->vk::UniqueDevice::operator=(physical_device.createDeviceUnique
			(vk::DeviceCreateInfo{
				.flags{vk::DeviceCreateFlags()},
				.queueCreateInfoCount{static_cast<uint32_t>(queue_create_info.size())},
				.pQueueCreateInfos{queue_create_info.data()},
				.enabledLayerCount{static_cast<uint32_t>(details::validation_layers::enabled ? details::validation_layers::list.size() : 0)},
				.ppEnabledLayerNames{details::validation_layers::enabled ? details::validation_layers::list.data() : nullptr},
				.enabledExtensionCount{static_cast<uint32_t>(device_extensions.size())},
				.ppEnabledExtensionNames{device_extensions.data()},
				.pEnabledFeatures{&device_features},
				}));
			}
		catch (vk::SystemError err) {
			throw error("Failed to create logical device!");
			}
		}
	}