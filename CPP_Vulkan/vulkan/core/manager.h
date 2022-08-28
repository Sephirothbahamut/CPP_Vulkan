#pragma once

#include <utils/compilation/debug.h>
#include <utils/memory.h>

#include "../dependencies.h"

#include "instance.h"
#include "debug_messenger.h"
#include "device.h"
#include "queues.h"
#include "swapchain_chosen_details.h"

namespace utils::graphics::vulkan::window { class window; class swapchain; class window_sized_images; }
namespace utils::graphics::vulkan::core
	{
	class manager
		{
		public:
			manager();

		private:
			instance instance;
			vk::DispatchLoaderDynamic dld;
			utils_if_debug(debug_messenger debug_messenger);
			swapchain_chosen_details swapchain_chosen_details;
			physical_device physical_device;
			device device;
			queues queues;

#pragma region getters

		public:
			class getter_window
				{
				friend class window::window;
				friend class manager;

				getter_window(const manager& manager);

				const vk::Instance& instance() const noexcept;

				utils::observer_ptr<const manager> manager_ptr;
				};
			class getter_swapchain
				{
				friend class window::swapchain;
				friend class manager;

				getter_swapchain(const manager& manager);

				const vk::PhysicalDevice& physical_device() const noexcept;
				const vk::Device& device() const noexcept;
				const core::swapchain_chosen_details& swapchain_chosen_details() const noexcept;
				const core::queues& queues() const noexcept;

				utils::observer_ptr<const manager> manager_ptr;
				};
			class getter_window_sized_images
				{
				friend class window::window_sized_images;
				friend class manager;

				getter_window_sized_images(const manager& manager);

				const vk::Device& device() const noexcept;

				utils::observer_ptr<const manager> manager_ptr;
				};
			friend class getter_window;
			friend class getter_swapchain;
			friend class getter_window_sized_images;
#pragma endregion
			
			inline getter_window              getter(const window::window             *) const noexcept { return { *this }; }
			inline getter_swapchain           getter(const window::swapchain          *) const noexcept { return { *this }; }
			inline getter_window_sized_images getter(const window::window_sized_images*) const noexcept { return { *this }; }
		};
	}