#pragma once

#include <utils/memory.h>
#include <utils/containers/handled_container.h>

#include "../dependencies.h"

#include "../core/manager.h"
#include "../core/image.h"

namespace utils::graphics::vulkan::window 
	{
	class window;
	class window_sized_image
		{
		public:
			window_sized_image(const core::manager& manager, const core::image::create_info& create_info, const core::image::create_view_info& create_view_info,
				const vk::MemoryPropertyFlags required_properties, const vk::Extent3D& extent);

			void resize(const core::manager& manager, const vk::Extent3D& new_extent);

			const core::image& get() const noexcept;

		private:
			core::image::create_info        saved_create_info  ;
			core::image::create_view_info   saved_create_view_info;
			vk::MemoryPropertyFlags         required_properties;
			core::image image;
		};

	class window_sized_images : public utils::containers::handled_container<window_sized_image>
		{
		friend class vulkan::window::window;
		public:
			using container_t = utils::containers::handled_container<window_sized_image>;
			window_sized_images(const core::manager& manager, const vk::Extent3D extent);

			container_t::handle_t emplace(const core::image::create_info& create_info, const core::image::create_view_info& create_view_info, const vk::MemoryPropertyFlags required_properties);

		private:
			const utils::observer_ptr<const vulkan::core::manager> manager_ptr;
			vk::Extent3D last_extent;

			void update_images(const vk::Extent3D& extent);
		};
	}