#include "manager.h"

namespace utils::graphics::vulkan::core
	{
		manager::manager() :
			instance           { "name", debug_messenger::create_info() },
			dld                { instance.get(), vkGetInstanceProcAddr },
#ifdef utils_is_debug
			debug_messenger    { instance.get(), dld },
#endif
			physical_device    { instance.get(), /* out */ swapchain_chosen_details},
			device             { physical_device, physical_device.get_graphics_family_index(), physical_device.get_present_family_index() },
			queues             { device.get(),    physical_device.get_graphics_family_index(), physical_device.get_present_family_index() },
			flying_frames_pool { device.get(), queues }
			{}

		// getter_window
		manager::getter_window::getter_window(const manager& manager) : manager_ptr{ &manager } {}

		const vk::Instance& manager::getter_window::instance() const noexcept
			{
			return manager_ptr->instance.get();
			}

		const vk::Device& manager::getter_window::device() const noexcept
			{
			return manager_ptr->device.get();
			}

		// getter_swapchain
		manager::getter_swapchain::getter_swapchain(const manager& manager) : manager_ptr{ &manager } {}

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

		// getter_window_sized_images
		manager::getter_window_sized_images::getter_window_sized_images(const manager& manager) : manager_ptr{ &manager } {}

		const vk::Device& manager::getter_window_sized_images::device() const noexcept
			{
			return manager_ptr->device.get();
			}

		// getter_rectangle_renderer
		manager::getter_rectangle_renderer::getter_rectangle_renderer(manager& manager) : manager_ptr{ &manager } {}

		const vk::Device& manager::getter_rectangle_renderer::device() const noexcept
			{
			return manager_ptr->device.get();
			}

		const swapchain_chosen_details& manager::getter_rectangle_renderer::swapchain_chosen_details() const noexcept
			{
			return manager_ptr->swapchain_chosen_details;
			}

		const queues& manager::getter_rectangle_renderer::queues() const noexcept
			{
			return manager_ptr->queues;
			}

		flying_frames_pool& manager::getter_rectangle_renderer::flying_frames_pool() noexcept
			{
			return manager_ptr->flying_frames_pool;
			}

		// getter_rectangle_renderer_const
		manager::getter_rectangle_renderer_const::getter_rectangle_renderer_const(const manager& manager) : manager_ptr{ &manager } {}

		const vk::Device& manager::getter_rectangle_renderer_const::device() const noexcept
			{
			return manager_ptr->device.get();
			}

		const swapchain_chosen_details& manager::getter_rectangle_renderer_const::swapchain_chosen_details() const noexcept
			{
			return manager_ptr->swapchain_chosen_details;
			}

		const queues& manager::getter_rectangle_renderer_const::queues() const noexcept
			{
			return manager_ptr->queues;
			}

		// getter_command_buffer_manager
		const vk::Device& manager::getter_flying_frames_pool::device() const noexcept
			{
			return manager_ptr->device.get();
			}

		const queues& manager::getter_flying_frames_pool::queues() const noexcept
			{
			return manager_ptr->queues;
			}
	}