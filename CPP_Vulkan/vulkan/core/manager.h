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

namespace utils::graphics::vulkan::core
	{
	class manager
		{
		public:
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
		private:
			instance instance;
			vk::DispatchLoaderDynamic dld;
			utils_if_debug(debug_messenger debug_messenger);
			swapchain_chosen_details swapchain_chosen_details;
			physical_device physical_device;
			device device;
			queues queues;
			flying_frames_pool flying_frames_pool;

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
			class getter_window_sized_images
				{
				friend class window::window_sized_images;
				friend class manager;

				getter_window_sized_images(const manager& manager);

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

			class getter_rectangle_renderer
				{
				friend class vulkan::renderer::rectangle_renderer;
				friend class manager;

				getter_rectangle_renderer(manager& manager);

				const vk::Device& device() const noexcept;
				const core::swapchain_chosen_details& swapchain_chosen_details() const noexcept;
				const core::queues& queues() const noexcept;
				core::flying_frames_pool& flying_frames_pool() noexcept;

				utils::observer_ptr<manager> manager_ptr;
				};
			class getter_rectangle_renderer_const
				{
				friend class vulkan::renderer::rectangle_renderer;
				friend class manager;

				getter_rectangle_renderer_const(const manager& manager);

				const vk::Device& device() const noexcept;
				const core::swapchain_chosen_details& swapchain_chosen_details() const noexcept;
				const core::queues& queues() const noexcept;

				utils::observer_ptr<const manager> manager_ptr;
				};
			class getter_renderer_3d
				{
				friend class vulkan::renderer::renderer_3d;
				friend class manager;

				getter_renderer_3d(manager& manager);

				const vk::PhysicalDevice& physical_device() const noexcept;
				const vk::Device& device() const noexcept;
				const core::swapchain_chosen_details& swapchain_chosen_details() const noexcept;
				const core::queues& queues() const noexcept;
				core::flying_frames_pool& flying_frames_pool() noexcept;

				utils::observer_ptr<manager> manager_ptr;
				};
			class getter_renderer_3d_const
				{
				friend class vulkan::renderer::renderer_3d;
				friend class manager;

				getter_renderer_3d_const(const manager& manager);

				const vk::PhysicalDevice& physical_device() const noexcept;
				const vk::Device& device() const noexcept;
				const core::swapchain_chosen_details& swapchain_chosen_details() const noexcept;
				const core::queues& queues() const noexcept;

				utils::observer_ptr<const manager> manager_ptr;
				};

			friend class getter_window;
			friend class getter_swapchain;
			friend class getter_window_sized_images;
			friend class getter_rectangle_renderer;
			friend class getter_rectangle_renderer_const;
			friend class getter_renderer_3d;
			friend class getter_renderer_3d_const;
			friend class getter_flying_frames_pool;
#pragma endregion

			inline getter_window                   getter(const window          ::window*)              const noexcept { return { *this }; }
			inline getter_swapchain                getter(const window          ::swapchain*)           const noexcept { return { *this }; }
			inline getter_window_sized_images      getter(const window          ::window_sized_images*) const noexcept { return { *this }; }
			inline getter_flying_frames_pool       getter(const core            ::flying_frames_pool*)  const noexcept { return { *this }; }
			inline getter_rectangle_renderer       getter(const vulkan::renderer::rectangle_renderer*)        noexcept { return { *this }; }
			inline getter_rectangle_renderer_const getter(const vulkan::renderer::rectangle_renderer*)  const noexcept { return { *this }; }
			inline getter_renderer_3d              getter(const vulkan::renderer::renderer_3d*)               noexcept { return { *this }; }
			inline getter_renderer_3d_const        getter(const vulkan::renderer::renderer_3d*)         const noexcept { return { *this }; }

			
		};
	}