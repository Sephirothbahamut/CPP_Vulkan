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
namespace utils::graphics::vulkan::renderer { class rectangle_renderer; }

namespace utils::graphics::vulkan::core
	{
	class manager
		{
		public:
			manager();
			~manager();

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
			class getter_rectangle_renderer
				{
				friend class renderer::rectangle_renderer;
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
				friend class renderer::rectangle_renderer;
				friend class manager;

				getter_rectangle_renderer_const(const manager& manager);

				const vk::Device& device() const noexcept;
				const core::swapchain_chosen_details& swapchain_chosen_details() const noexcept;
				const core::queues& queues() const noexcept;

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

			friend class getter_window;
			friend class getter_swapchain;
			friend class getter_window_sized_images;
			friend class getter_rectangle_renderer;
			friend class getter_rectangle_renderer_const;
			friend class getter_flying_frames_pool;
#pragma endregion

			inline getter_window                     getter(const window  ::window*)              const noexcept { return { *this }; }
			inline getter_swapchain                  getter(const window  ::swapchain*)           const noexcept { return { *this }; }
			inline getter_window_sized_images        getter(const window  ::window_sized_images*) const noexcept { return { *this }; }
			inline getter_rectangle_renderer         getter(const renderer::rectangle_renderer*)        noexcept { return { *this }; }
			inline getter_rectangle_renderer_const   getter(const renderer::rectangle_renderer*)  const noexcept { return { *this }; }
			inline getter_flying_frames_pool         getter(const core    ::flying_frames_pool*)  const noexcept { return { *this }; }

			/*enum class get_id_t { instance, physical_device, device, swapchain_chosen_details, queues };
		public:
			template<typename T, get_id_t... get_ids>
			class getter_t
				{
				friend class T;
				friend class manager;

				getter_t(const manager& manager) : manager_ptr{&manager}{}

				template<std::enable_if <utils::variadic::contains_value < get_id_t, get_id_t::instance, get_ids...>::value>>
				const vk::Instance& instance() const noexcept
					{
					return manager_ptr->instance.get();
					}

				template<std::enable_if <utils::variadic::contains_value < get_id_t, get_id_t::physical_device, get_ids...>::value>>
				const vk::PhysicalDevice& physical_device() const noexcept
					{
					return manager_ptr->physical_device;
					}

				template<get_id_t ...get_ids = get_ids..., std::enable_if <utils::variadic::contains_value < get_id_t, get_id_t::device, get_ids...>::value>>
				const vk::Device& device() const noexcept
					{
					return manager_ptr->device.get();
					}

				template<std::enable_if <utils::variadic::contains_value < get_id_t, get_id_t::swapchain_chosen_details, get_ids...>::value>>
				const core::swapchain_chosen_details& swapchain_chosen_details() const noexcept
					{
					return manager_ptr->swapchain_chosen_details;
					}

				template<std::enable_if <utils::variadic::contains_value < get_id_t, get_id_t::queues, get_ids...>::value>>
				const vk::Queue& queues() const noexcept
					{
					return manager_ptr->queues;
					}

				utils::observer_ptr<const manager> manager_ptr;
				};
		
			inline getter_t <window::window, get_id_t::instance> 
				getter(const window::window*) const noexcept { return { *this }; }

			inline getter_t <window::swapchain, get_id_t::physical_device, get_id_t::device, get_id_t::swapchain_chosen_details, get_id_t::queues>     
				getter(const window::swapchain*) const noexcept { return { *this }; }

			inline getter_t <window::window_sized_images, get_id_t::device>
				getter(const window::window_sized_images*) const noexcept { return { *this }; }

			inline getter_t <renderer::rectangle_renderer, get_id_t::device, get_id_t::swapchain_chosen_details>
				getter(const renderer::rectangle_renderer*) const noexcept { return { *this }; }*/
		};
	}