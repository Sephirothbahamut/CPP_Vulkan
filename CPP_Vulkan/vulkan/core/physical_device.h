#pragma once

#include <Windows.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_win32.h>

#include "utils.h"
#include "instance.h"
#include "debug_messenger.h"

namespace utils::graphics::vulkan
	{
	namespace details
		{
		class proxy_surface
			{
			public:
				inline proxy_surface(const vk::Instance& instance) : instance(&instance)
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

				proxy_surface(const proxy_surface& copy) = delete;
				proxy_surface& operator=(const proxy_surface& copy) = delete;

				inline const vk::SurfaceKHR& get() const
					{
					return surface;
					}

				inline ~proxy_surface()
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

			[[nodiscard]] inline static std::vector<const char*> check_missing(const vk::PhysicalDevice& physical_device)
				{
				auto available{physical_device.enumerateDeviceExtensionProperties()};
				return check_required_in_available(get_required(), available, [](const auto& required, const auto& available) { return strcmp(required, available.extensionName) == 0; });
				}

			};

		inline float evaluate_device(const vk::PhysicalDevice& physical_device, const proxy_surface& proxy_surface)
			{
			auto missing_device_extensions{device_extensions::check_missing(physical_device)};

			//TODO actually calculate the scoring fr (thru features / properties )
			bool queue_families_incomplete{!queue_families{ physical_device }.is_complete()};
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
					using namespace utils::cout;
					message << "missing the following device extensions:\n";
					message << missing_device_extensions;
					}
				logger.log(message.str());
				return {0.f, std::nullopt};
				}

			// Must be checked after ensuring extensions' availability
			auto swap_chain_info{get_swap_chain_info(physical_device, proxy_surface)};
			bool swap_chain_inadequate{swap_chain_info.formats.empty() || swap_chain_info.present_modes.empty()};

			if (swap_chain_inadequate)
				{
				auto device_properties{physical_device.getProperties()};
				std::stringstream message{"Device "};
				message << device_properties.deviceName << " swap chain inadequate: missing formats or present modes";
				logger.log(message.str());
				return {0.f, std::nullopt};
				}

			return {1.f, swap_chain_info};
			}

		inline swap_chain_support_details get_swap_chain_info(vk::PhysicalDevice vk_physical_device, const proxy_surface& surface)
			{
			return
				{
					.capabilities{vk_physical_device.getSurfaceCapabilitiesKHR(surface.get())},
					.formats{vk_physical_device.getSurfaceFormatsKHR(surface.get())},
					.present_modes{vk_physical_device.getSurfacePresentModesKHR(surface.get())}
				};
			}
		
		}


	}