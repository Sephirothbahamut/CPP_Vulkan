#include "image.h"

#include "utils.h"

namespace utils::graphics::vulkan::core
	{
	image::image(const manager& manager, const create_info& create_info, const vk::Extent3D& extent) :
		vk_unique_image      { create_image   (manager, create_info.image, extent) },
		vk_unique_memory     { allocate_memory(manager, create_info.memory_properties) }
		{
		manager.get_device().bindImageMemory(vk_unique_image.get(), vk_unique_memory.get(), 0);
		vk_unique_image_view = create_image_view(manager, create_info.view);
		}

	const vk::Image& image::get() const noexcept
		{
		return vk_unique_image.get();
		}

	const vk::ImageView& image::view() const noexcept
		{
		return vk_unique_image_view.get();
		}

	const vk::Image& image::operator*() const noexcept
		{
		return vk_unique_image.get();
		}

	const vk::Image* image::operator->() const noexcept
		{
		return &vk_unique_image.get();
		}

	vk::UniqueImage image::create_image(const manager& manager, const create_info::image_t& create_info, const vk::Extent3D& extent) const
		{
		return manager.get_device().createImageUnique(
			vk::ImageCreateInfo
			{
				.flags{ create_info.flags },
				.imageType{ create_info.image_type },
				.format{ create_info.format },
				.extent{ extent },
				.mipLevels{ create_info.mip_levels },
				.arrayLayers{ create_info.array_layers },
				.samples{ create_info.samples },
				.tiling{ create_info.tiling },
				.usage{ create_info.usage },
				.sharingMode{ create_info.sharing_mode },
				.queueFamilyIndexCount{ static_cast<uint32_t>(create_info.queue_family_indices.size()) },
				.pQueueFamilyIndices{ create_info.queue_family_indices.data()},
				.initialLayout{ create_info.initial_layout },
			});
		}

	vk::UniqueDeviceMemory image::allocate_memory(const manager& manager, const vk::MemoryPropertyFlags required_properties) const
		{
		auto& device{ manager.get_device() };
		vk::UniqueDeviceMemory ret;

		vk::MemoryRequirements mem_requirements;
		mem_requirements = device.getImageMemoryRequirements(vk_unique_image.get());

		vk::MemoryAllocateInfo allocate_info
			{
			.allocationSize { mem_requirements.size },
			.memoryTypeIndex{ details::find_memory_type(manager.get_physical_device(), mem_requirements.memoryTypeBits, required_properties)},
			};

		return device.allocateMemoryUnique(allocate_info, nullptr);
		}

	vk::UniqueImageView image::create_image_view(const manager& manager, const create_info::view_t& create_view_info) const
		{
		return manager.get_device().createImageViewUnique(
			{
			.image            { vk_unique_image.get()      },
			.viewType         { create_view_info.view_type },
			.format           { create_view_info.format    },
			.subresourceRange { create_view_info.subresource_range },
			});
		}
	}

