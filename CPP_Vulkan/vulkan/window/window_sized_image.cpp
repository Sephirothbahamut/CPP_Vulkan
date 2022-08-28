#pragma once
#include "window_sized_image.h"

#include "../window/window.h"

namespace utils::graphics::vulkan::window
	{
	window_sized_image::window_sized_image(const vk::Device& device, const create_info& create_info, const vk::Extent3D& extent) :
				vk_unique_image{ create(device, create_info, extent) }, saved_create_info{ create_info }{}

	void window_sized_image::resize(const vk::Device& device, const vk::Extent3D& new_extent)
		{
		vk_unique_image = create(device, saved_create_info, new_extent);
		}

	const vk::Image& window_sized_image::get() const noexcept
		{
		return vk_unique_image.get();
		}

	const vk::Image& window_sized_image::operator*() const noexcept
		{
		return vk_unique_image.get();
		}

	const vk::Image* window_sized_image::operator->() const noexcept
		{
		return &vk_unique_image.get();
		}

	vk::UniqueImage window_sized_image::create(const vk::Device& device, const create_info& create_info, const vk::Extent3D& extent)
		{
		return device.createImageUnique(
			vk::ImageCreateInfo
			{
				.flags{ create_info.flags },
				.imageType{ create_info.imageType },
				.format{ create_info.format },
				.extent{ extent },
				.mipLevels{ create_info.mipLevels },
				.arrayLayers{ create_info.arrayLayers },
				.samples{ create_info.samples },
				.tiling{ create_info.tiling },
				.usage{ create_info.usage },
				.sharingMode{ create_info.sharingMode },
				.queueFamilyIndexCount{ create_info.queueFamilyIndexCount },
				.pQueueFamilyIndices{ create_info.pQueueFamilyIndices },
				.initialLayout{ create_info.initialLayout },
			});
		}

	window_sized_images::window_sized_images(const core::manager& manager, const vk::Extent3D extent) :
				manager_ptr{ &manager }, last_extent{ extent }
		{}

	window_sized_images::container_t::handle_t window_sized_images::emplace(const window_sized_image::create_info& create_info)
		{
		return container_t::emplace(manager_ptr->getter(this).device(), create_info, last_extent);
		}

	void window_sized_images::update_images(const vk::Extent3D& new_extent)
		{
		for (auto& image : *this)
			{
			image.resize(manager_ptr->getter(this).device(), new_extent);
			}
		last_extent = new_extent;
		}

	};