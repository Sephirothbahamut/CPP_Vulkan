#include "manager.h"

namespace utils::graphics::vulkan::core
	{
		manager::manager() :
			instance{ "name", debug_messenger::create_info() },
			dld{ instance.get(), vkGetInstanceProcAddr },
#ifdef utils_is_debug
			debug_messenger{ instance.get(), dld },
#endif
			physical_device{ instance.get(), /* out */ swapchain_chosen_details},
			device{ physical_device, physical_device.get_graphics_family_index(), physical_device.get_present_family_index() },
			queues{    device.get(), physical_device.get_graphics_family_index(), physical_device.get_present_family_index() }
			{}
			
		manager::getter_window::getter_window(const manager& manager) : manager_ptr{ &manager } {}
		manager::getter_swapchain::getter_swapchain(const manager& manager) : manager_ptr{ &manager } {}

		const vk::Instance& manager::getter_window::instance() const noexcept
			{
			return manager_ptr->instance.get();
			}

		const vk::PhysicalDevice& manager::getter_swapchain::physical_device() const noexcept
			{
			return manager_ptr->physical_device;
			}

		const vk::Device& manager::getter_swapchain::device() const noexcept
			{
			return manager_ptr->device.get();
			}

		const swapchain_chosen_details& manager::getter_swapchain::swapchain_chosen_details() const noexcept
			{
			return manager_ptr->swapchain_chosen_details;
			}

		const queues& manager::getter_swapchain::queues() const noexcept
			{
			return manager_ptr->queues;
			}
	}