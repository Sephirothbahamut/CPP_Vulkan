#include "manager.h"

#include "utils.h"

namespace utils::graphics::vulkan::core
	{
		manager::manager() :
			instance                         { "name", debug_messenger::create_info() },
			dld                              { instance.get(), vkGetInstanceProcAddr },
#ifdef utils_is_debug		                 
			debug_messenger                  { instance.get(), dld },
#endif						                 
			physical_device                  { instance.get(), /* out */ swapchain_chosen_details},
			device                           { physical_device, physical_device.get_graphics_family_index(), physical_device.get_present_family_index() },
			queues                           { device.get(),    physical_device.get_graphics_family_index(), physical_device.get_present_family_index() },
			flying_frames_pool               { device.get(), queues },
			vk_unique_memory_op_command_pool { create_memory_op_command_pool() }
			{}
		
		void manager::bind(window::window& window, renderer& renderer)
			{
			std::unique_ptr<renderer_window_data> tmp{ new renderer_window_data{*this, renderer, window} };

			renderer.window_dependent_data_ptrs.emplace(&window, tmp.get());
			window.renderer_dependent_data_ptrs.emplace_back(tmp.release());
			}

		vk::UniqueCommandPool manager::create_memory_op_command_pool()
		{
			try
				{
				return device.get().createCommandPoolUnique
				({
					.flags{vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient},
					.queueFamilyIndex{queues.get_graphics().index},
					});
				}
			catch (vk::SystemError err)
				{
				throw std::runtime_error{ "Failed to create command pool!" };
				}
		}

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

		// getter_image

		manager::getter_image::getter_image(const manager& manager) : manager_ptr{ &manager } {}

		const vk::PhysicalDevice& manager::getter_image::physical_device() const noexcept
			{
			return manager_ptr->physical_device;
			}

		const vk::Device& manager::getter_image::device() const noexcept
			{
			return manager_ptr->device.get();
			}

		// getter_flying_frames_pool

		manager::getter_flying_frames_pool::getter_flying_frames_pool(const manager& manager) : manager_ptr{ &manager } {}

		const vk::Device& manager::getter_flying_frames_pool::device() const noexcept
			{
			return manager_ptr->device.get();
			}

		const queues& manager::getter_flying_frames_pool::queues() const noexcept
			{
			return manager_ptr->queues;
			}

		// getter_renderer

		manager::getter_renderer::getter_renderer(manager& manager) : manager_ptr{ &manager } {}

		const vk::PhysicalDevice& manager::getter_renderer::physical_device() const noexcept
			{
			return manager_ptr->physical_device;
			}

		const vk::Device& manager::getter_renderer::device() const noexcept
			{
			return manager_ptr->device.get();
			}

		const core::swapchain_chosen_details& manager::getter_renderer::swapchain_chosen_details() const noexcept
			{
			return manager_ptr->swapchain_chosen_details;
			}

		const core::queues& manager::getter_renderer::queues() const noexcept
			{
			return manager_ptr->queues;
			}

		core::flying_frames_pool& manager::getter_renderer::flying_frames_pool() noexcept
			{
			return manager_ptr->flying_frames_pool;
			}

		vk::CommandPool& manager::getter_renderer::memory_op_command_pool() noexcept
			{
			return manager_ptr->vk_unique_memory_op_command_pool.get();
			}

		// getter_renderer_const

		manager::getter_renderer_const::getter_renderer_const(const manager& manager) : manager_ptr{ &manager } {}

		const vk::PhysicalDevice& manager::getter_renderer_const::physical_device() const noexcept
			{
			return manager_ptr->physical_device;
			}
		const vk::Device& manager::getter_renderer_const::device() const noexcept
			{
			return manager_ptr->device.get();
			}
		const core::swapchain_chosen_details& manager::getter_renderer_const::swapchain_chosen_details() const noexcept
			{
			return manager_ptr->swapchain_chosen_details;
			}
		const core::queues& manager::getter_renderer_const::queues() const noexcept
			{
			return manager_ptr->queues;
			}

		// getter_renderer_window_data

		manager::getter_renderer_window_data::getter_renderer_window_data(const manager& manager) : manager_ptr{ &manager } {}

		const vk::Device& manager::getter_renderer_window_data::device() const noexcept
			{
			return manager_ptr->device.get();
			}
}