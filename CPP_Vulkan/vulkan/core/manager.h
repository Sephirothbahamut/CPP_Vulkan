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

namespace utils::graphics::vulkan::window   { class window; class swapchain; class window_sized_images; }
namespace utils::graphics::vulkan::renderer { class rectangle_renderer; class renderer_3d; }
namespace utils::graphics::vulkan::core     { class image; class renderer_window_data; class renderer; }

namespace utils::graphics::vulkan::core
	{
	class manager
		{
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

			vk::Instance&                   get_instance()                 noexcept { return instance.get(); }
			core::swapchain_chosen_details& get_swapchain_chosen_details() noexcept { return swapchain_chosen_details; };
			
			vk::PhysicalDevice&             get_physical_device()          noexcept { return physical_device; }
			vk::Device&                     get_device()                   noexcept { return device.get(); }

			queues&                         get_queues()                   noexcept { return queues; }
			core::flying_frames_pool&       get_flying_frames_pool()       noexcept { return flying_frames_pool; }
			vk::CommandPool&                get_memory_op_command_pool()   noexcept { return vk_unique_memory_op_command_pool.get(); }
			

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
#pragma region getters

		public:
			class getter_window
				{
				friend class window::window;
				friend class manager;

				getter_window(const manager& manager);

				const vk::Instance& instance() const noexcept;
				const vk::Device& device() const noexcept;

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
			class getter_image
				{
				friend class core::image;
				friend class manager;

				getter_image(const manager& manager);

				const vk::PhysicalDevice& physical_device() const noexcept;
				const vk::Device& device() const noexcept;

				utils::observer_ptr<const manager> manager_ptr;
				};
			class getter_flying_frames_pool
				{
				friend class flying_frames_pool;
				friend class manager;

				getter_flying_frames_pool(const manager& manager);

				const vk::Device& device() const noexcept;
				const core::queues& queues() const noexcept;

				utils::observer_ptr<const manager> manager_ptr;
				};
			class getter_renderer
				{
				friend class vulkan::core::renderer;
				friend class manager;

				getter_renderer(manager& manager);
				
				public:
					const vk::PhysicalDevice& physical_device() const noexcept;
					const vk::Device& device() const noexcept;
					const core::swapchain_chosen_details& swapchain_chosen_details() const noexcept;
					const core::queues& queues() const noexcept;
					core::flying_frames_pool& flying_frames_pool() noexcept;
					vk::CommandPool& memory_op_command_pool() noexcept;
				private:
					utils::observer_ptr<manager> manager_ptr;
				};
			class getter_renderer_const
				{
				friend class vulkan::core::renderer;
				friend class manager;

				getter_renderer_const(const manager& manager);

				public:
					const vk::PhysicalDevice& physical_device() const noexcept;
					const vk::Device& device() const noexcept;
					const core::swapchain_chosen_details& swapchain_chosen_details() const noexcept;
					const core::queues& queues() const noexcept;
				private:
					utils::observer_ptr<const manager> manager_ptr;
				};
			class getter_renderer_window_data
				{
				friend class renderer_window_data;
				friend class manager;

				getter_renderer_window_data(const manager& manager);

				const vk::Device& device() const noexcept;

				utils::observer_ptr<const manager> manager_ptr;
				};
			
			friend class getter_window;
			friend class getter_swapchain;
			friend class getter_image;
			friend class getter_flying_frames_pool;
			friend class getter_renderer;
			friend class getter_renderer_const;
			friend class getter_renderer_window_data;
#pragma endregion

			inline getter_window                   getter(const window          ::window*)                const noexcept { return { *this }; }
			inline getter_swapchain                getter(const window          ::swapchain*)             const noexcept { return { *this }; }
			inline getter_image                    getter(const core            ::image*)                 const noexcept { return { *this }; }
			inline getter_flying_frames_pool       getter(const core            ::flying_frames_pool*)    const noexcept { return { *this }; }
			inline getter_renderer                 getter(      core            ::renderer*)                    noexcept { return { *this }; }
			inline getter_renderer_const           getter(const core            ::renderer*)              const noexcept { return { *this }; }
			inline getter_renderer_window_data     getter(const core            ::renderer_window_data*)  const noexcept { return { *this }; }
			
		};
	}