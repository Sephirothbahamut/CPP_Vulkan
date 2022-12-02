#pragma once

#include <utils/compilation/debug.h>
#include <utils/memory.h>
#include <utils/variadic.h>

#include "../dependencies.h"

#include "instance.h"
#include "debug_messenger.h"
#include "device.h"
#include "queues.h"
#include "swapchain_chosen_details.h"
#include "flying_frames_pool.h"

namespace utils::graphics::vulkan::window { class window; class swapchain; class window_sized_images; }
namespace utils::graphics::vulkan::core   { class image; class renderer_window_data; class renderer; class frame; }

namespace utils::graphics::vulkan::core
	{
	class manager
		{
		friend class window::window;
		friend class window::swapchain;
		friend class core::image;
		friend class flying_frames_pool;
		friend class vulkan::core::renderer;
		friend class vulkan::core::frame;
		friend class renderer_window_data;

		public:
			void bind(window::window& window, renderer& renderer);

			class closer_t
				{
				friend class manager;
				public:
					~closer_t() { manager_ptr->device->waitIdle(); }

				private:
					closer_t(manager& manager) : manager_ptr{&manager} {}
					utils::observer_ptr<manager> manager_ptr;
				};

			manager();

			[[nodiscard]] closer_t get_closer()
				{
				return closer_t{*this};
				}
			
			      vk::Instance&                   get_instance()                       noexcept { return instance.get(); }
			const vk::Instance&                   get_instance()                 const noexcept { return instance.get(); }
			      core::swapchain_chosen_details& get_swapchain_chosen_details()       noexcept { return swapchain_chosen_details; };
			const core::swapchain_chosen_details& get_swapchain_chosen_details() const noexcept { return swapchain_chosen_details; };
			
			      vk::PhysicalDevice&             get_physical_device()                noexcept { return physical_device; }
			const vk::PhysicalDevice&             get_physical_device()          const noexcept { return physical_device; }
			      vk::Device&                     get_device()                         noexcept { return device.get(); }
			const vk::Device&                     get_device()                   const noexcept { return device.get(); }
			
			      queues&                         get_queues()                         noexcept { return queues; }
			const queues&                         get_queues()                   const noexcept { return queues; }
			      core::flying_frames_pool&       get_flying_frames_pool()             noexcept { return flying_frames_pool; }
			const core::flying_frames_pool&       get_flying_frames_pool()       const noexcept { return flying_frames_pool; }
			      vk::CommandPool&                get_memory_op_command_pool()         noexcept { return vk_unique_memory_op_command_pool.get(); }
			const vk::CommandPool&                get_memory_op_command_pool()   const noexcept { return vk_unique_memory_op_command_pool.get(); }
			

		private:
			instance instance;
			vk::DispatchLoaderDynamic dld;
			utils_if_debug(debug_messenger debug_messenger);
			swapchain_chosen_details swapchain_chosen_details;
			physical_device physical_device;
			device device;
			queues queues;
			flying_frames_pool flying_frames_pool;
			vk::UniqueCommandPool vk_unique_memory_op_command_pool;

			vk::UniqueCommandPool create_memory_op_command_pool();
		};
	}