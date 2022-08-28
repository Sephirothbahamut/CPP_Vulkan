#pragma once

#include <utils/memory.h>
#include <utils/containers/handled_container.h>

#include "../dependencies.h"

#include "../core/manager.h"

namespace utils::graphics::vulkan::window 
	{
	class window;
	class window_sized_image
		{
		public:

			struct create_info
				{
				vk::ImageCreateFlags    flags{};
				vk::ImageType           imageType{ vk::ImageType::e1D };
				vk::Format              format{ vk::Format::eUndefined };
				uint32_t                mipLevels{};
				uint32_t                arrayLayers{};
				vk::SampleCountFlagBits samples{ vk::SampleCountFlagBits::e1 };
				vk::ImageTiling         tiling{ vk::ImageTiling::eOptimal };
				vk::ImageUsageFlags     usage{};
				vk::SharingMode         sharingMode{ vk::SharingMode::eExclusive };
				uint32_t                queueFamilyIndexCount{};
				const uint32_t* pQueueFamilyIndices{};
				vk::ImageLayout         initialLayout{ vk::ImageLayout::eUndefined };
				};

			window_sized_image(const vk::Device& device, const create_info& create_info, const vk::Extent3D& extent);

			void resize(const vk::Device& device, const vk::Extent3D& new_extent);

			const vk::Image& get() const noexcept;

			const vk::Image& operator*() const noexcept;

			const vk::Image* operator->() const noexcept;

		private:
			create_info saved_create_info;
			vk::UniqueImage vk_unique_image;

			vk::UniqueImage create(const vk::Device& device, const create_info& create_info, const vk::Extent3D& extent);
		};

	class window_sized_images : public utils::containers::handled_container<window_sized_image>
		{
		friend class vulkan::window::window;
		public:
			using container_t = utils::containers::handled_container<window_sized_image>;
			window_sized_images(const core::manager& manager, const vk::Extent3D extent);

			container_t::handle_t emplace(const window_sized_image::create_info& create_info);

		private:
			const utils::observer_ptr<const vulkan::core::manager> manager_ptr;
			vk::Extent3D last_extent;

			void update_images(const vk::Extent3D& extent);
		};

	/* practicing bad practices(it works for now tho)
	class window_sized_images : public std::vector<window_sized_image>
		{
		public:
			using vector_t = std::vector<window_sized_image>;
			inline window_sized_images(const core::manager& manager, const vulkan::window::window& window) :
				manager_ptr{ &manager }, window_ptr{ &window } {}

			template<std::same_as<window_sized_image::create_info> ...create_info_ts>
			inline window_sized_images(const core::manager& manager, const vulkan::window::window& window, create_info_ts&&... create_infos) :
				manager_ptr{ &manager },
				window_ptr { &window  },
				vector_t{ window_sized_image{manager.getter(this).device(), create_infos, window.extent}... } 
			{}

			window_sized_image& emplace_back(const window_sized_image::create_info& create_info)
				{
				return vector_t::emplace_back(manager_ptr->getter(this).device(), create_info, window_ptr->extent);
				}

			vector_t::iterator emplace(vector_t::const_iterator it, const window_sized_image::create_info& create_info)
				{
				return vector_t::emplace(it, manager_ptr->getter(this).device(), create_info, window_ptr->extent);
				}

			void update_images()
				{
				for (auto& image : *this)
					{
					image.resize(manager_ptr->getter(this).device(), window_ptr->extent);
					}
				}
			
		private:
			const utils::observer_ptr<const vulkan::  core::manager> manager_ptr;
			const utils::observer_ptr<const vulkan::window::window > window_ptr ;
		};*/
	}