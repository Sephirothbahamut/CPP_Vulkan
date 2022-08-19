#pragma once

#include <utils/compilation/debug.h>
#include <utils/memory.h>

#include <vulkan/vulkan.hpp>

#include "instance.h"
#include "debug_messenger.h"
#include "device.h"
#include "queues.h"
#include "swapchain_chosen_details.h"

namespace utils::graphics::vulkan::window { class window; class swapchain; }
namespace utils::graphics::vulkan::core
	{
	class manager
		{
		public:
			manager();

			class getter_window
				{
				friend class window::window;

				getter_window(const manager& manager);

				const vk::Instance& instance() const noexcept;
				
				utils::observer_ptr<const manager> manager_ptr;
				};
			class getter_swapchain
				{
				friend class window::swapchain;

				getter_swapchain(const manager& manager);

				const vk::PhysicalDevice& physical_device() const noexcept;
				const vk::Device& device() const noexcept;
				const swapchain_chosen_details& swapchain_chosen_details() const noexcept;
				const queues& queues() const noexcept;

				utils::observer_ptr<const manager> manager_ptr;
				};
			friend class getter_window;
			friend class getter_swapchain;

		private:
			instance instance;
			vk::DispatchLoaderDynamic dld;
			utils_if_debug(debug_messenger debug_messenger);
			swapchain_chosen_details swapchain_chosen_details;
			physical_device physical_device;
			device device;
			queues queues;
		};
	}