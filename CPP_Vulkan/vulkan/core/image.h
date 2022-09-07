#pragma once

#include "../dependencies.h"

#include "manager.h"

namespace utils::graphics::vulkan::core
	{
	class image
		{
		public:
			struct create_info
				{
				vk::ImageCreateFlags    flags{};
				vk::ImageType           image_type{ vk::ImageType::e1D };
				vk::Format              format{ vk::Format::eUndefined };
				uint32_t                mip_levels{};
				uint32_t                array_layers{};
				vk::SampleCountFlagBits samples{ vk::SampleCountFlagBits::e1 };
				vk::ImageTiling         tiling{ vk::ImageTiling::eOptimal };
				vk::ImageUsageFlags     usage{};
				vk::SharingMode         sharing_mode{ vk::SharingMode::eExclusive };
				std::vector<uint32_t>   queue_family_indices{};
				vk::ImageLayout         initial_layout{ vk::ImageLayout::eUndefined };
				};
			
			image(const manager& manager, const create_info& create_info, const vk::MemoryPropertyFlags required_properties, const vk::Extent3D& extent);

			const vk::Image& get() const noexcept;

			const vk::Image& operator*() const noexcept;

			const vk::Image* operator->() const noexcept;

		private:
			vk::UniqueImage        vk_unique_image ;
			vk::UniqueDeviceMemory vk_unique_memory;

			vk::UniqueImage        create_image    (const manager& manager, const create_info& create_info, const vk::Extent3D& extent)          const;
			vk::UniqueDeviceMemory allocate_memory (const manager& manager, const vk::MemoryPropertyFlags required_properties)                         const;
		};
	}