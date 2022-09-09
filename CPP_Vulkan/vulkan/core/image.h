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
			struct create_view_info
				{
				vk::ImageViewType         view_type        { vk::ImageViewType::e1D };
				vk::Format                format           { vk::Format::eUndefined };
				vk::ComponentMapping      components       { {} };
				vk::ImageSubresourceRange subresource_range{ {} };
				};
			
			image(const manager& manager, const create_info& create_info, const create_view_info& create_view_info, const vk::MemoryPropertyFlags required_properties, const vk::Extent3D& extent);

			const vk::Image& get() const noexcept;

			const vk::ImageView& view() const noexcept;

			const vk::Image& operator*() const noexcept;

			const vk::Image* operator->() const noexcept;

		private:
			vk::UniqueImage        vk_unique_image ;
			vk::UniqueDeviceMemory vk_unique_memory;
			vk::UniqueImageView    vk_unique_image_view;

			vk::UniqueImage        create_image     (const manager& manager, const create_info& create_info, const vk::Extent3D& extent) const;
			vk::UniqueDeviceMemory allocate_memory  (const manager& manager, const vk::MemoryPropertyFlags required_properties)          const;
			vk::UniqueImageView    create_image_view(const manager& manager, const create_view_info& create_view_info)                   const;
		};
	}