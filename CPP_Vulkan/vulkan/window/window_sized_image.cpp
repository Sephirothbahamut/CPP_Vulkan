#pragma once
#include "window_sized_image.h"

#include "../window/window.h"

namespace utils::graphics::vulkan::window
	{
	window_sized_image::window_sized_image(const core::manager& manager, const core::image::create_info& create_info, const core::image::create_view_info& create_view_info,
		const vk::MemoryPropertyFlags required_properties, const vk::Extent3D& extent) :
			saved_create_info   { create_info   },
			saved_create_view_info { create_view_info },
			required_properties { required_properties },
			image               { manager, create_info, create_view_info, required_properties, extent } {}

	void window_sized_image::resize(const core::manager& manager, const vk::Extent3D& new_extent)
		{
		image = core::image{ manager, saved_create_info, saved_create_view_info, required_properties, new_extent };
		}

	const core::image& window_sized_image::get() const noexcept
		{
		return image;
		}

	window_sized_images::window_sized_images(const core::manager& manager, const vk::Extent3D extent) :
		manager_ptr{ &manager }, last_extent{ extent }
		{}

	window_sized_images::container_t::handle_t window_sized_images::emplace(const core::image::create_info& create_info,
		const core::image::create_view_info& create_view_info, const vk::MemoryPropertyFlags required_properties)
		{
		return container_t::emplace(*manager_ptr, create_info, create_view_info, required_properties, last_extent);
		}

	void window_sized_images::update_images(const vk::Extent3D& new_extent)
		{
		for (auto& image : *this)
			{
			image.resize(*manager_ptr, new_extent);
			}
		last_extent = new_extent;
		}

	};