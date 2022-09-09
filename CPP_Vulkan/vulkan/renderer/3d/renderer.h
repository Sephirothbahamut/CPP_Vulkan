#pragma once

#include "../../dependencies.h"

#include "../../core/utils.h"
#include "../../core/shader.h"
#include "../../core/renderer.h"
#include "../../core/model.h"
#include "../../window/window_sized_image.h"

#include <vulkan/vulkan.h>

namespace utils::graphics::vulkan::renderer
	{
	class renderer_3d : public core::renderer
		{
		public:
			renderer_3d(core::manager& manager, const Model& model);

			void resize(const core::manager& manager, const window::window& window) final override;

			void draw(core::manager& manager, const window::window& window, float delta_time) final override;

		private:
			using image_handle = window::window_sized_images::container_t::handle_t;

			core::shader_vertex                 vertex_shader            ;
			core::shader_fragment               fragment_shader          ;
			vk::UniqueRenderPass                vk_unique_renderpass     ;
			vk::UniquePipelineLayout            vk_unique_pipeline_layout;
			vk::UniquePipeline                  vk_unique_pipeline       ;
			core::image                         vk_depth_image           ;
			//vk::UniqueImageView                 vk_depth_image_view      ;
			//std::vector<image_handle>           image_handles            ;
			//std::vector<vk::UniqueImageView>    image_handles_views      ;
			std::vector<vk::UniqueFramebuffer>  vk_unique_framebuffers   ;

			vk::UniqueBuffer                    vk_unique_staging_vertex_buffer;
			vk::UniqueDeviceMemory              vk_unique_staging_vertex_memory;
			vk::UniqueBuffer                    vk_unique_vertex_buffer        ;
			vk::UniqueDeviceMemory              vk_unique_vertex_memory        ;
			vk::UniqueBuffer                    vk_unique_staging_index_buffer ;
			vk::UniqueDeviceMemory              vk_unique_staging_index_memory ;
			vk::UniqueBuffer                    vk_unique_index_buffer         ;
			vk::UniqueDeviceMemory              vk_unique_index_memory         ;
			size_t                              vertices_count                 ;
			size_t                              indices_count                  ;

			vk::UniqueRenderPass     create_renderpass     (const core::manager& manager) const;

			vk::UniquePipelineLayout create_pipeline_layout(const core::manager& manager) const;
		
			vk::UniquePipeline       create_pipeline       (const core::manager& manager, const vk::RenderPass& renderpass, const core::shader_vertex& vertex_shader, const core::shader_fragment& fragment_shader) const;
		
			// TODO distruttore di renderer deve distruggere le handle
			/*image_handle create_image(window::window& window, const core::image::create_info& create_info,
				const vk::MemoryPropertyFlags required_properties)
				{
					return window.images.emplace(create_info, required_properties);
				}

			std::vector<image_handle> create_images(window::window& window, const core::image::create_info& create_info,
				const vk::MemoryPropertyFlags required_properties, size_t amount)
				{
				std::vector<image_handle> ret;
				for (size_t i = 0; i < amount; i++)
					{
					ret.push_back(create_image(window, create_info, required_properties));
					}
				return ret;
				}

			vk::UniqueImageView create_image_view(const core::manager& manager, const vk::Image& vk_image, vk::Format format, vk::ImageAspectFlags aspect_flags)
				{
				return manager.getter(this).device().createImageViewUnique(
					{
					.image    { vk_image },
					.viewType { vk::ImageViewType::e2D },
					.format   { format },
					.subresourceRange
						{
						.aspectMask     { aspect_flags },
						.baseMipLevel   { 0 },
						.levelCount     { 1 },
						.baseArrayLayer { 0 },
						.layerCount     { 1 },
						},
					});
				}

			std::vector<vk::UniqueImageView> create_image_views(const core::manager& manager, const vk::Image& vk_image, vk::Format format, vk::ImageAspectFlags aspect_flags, size_t amount)
				{
				std::vector<vk::UniqueImageView> ret;
				for (size_t i = 0; i < amount; i++)
					{
					ret.push_back(create_image_view(manager, vk_image, format, aspect_flags));
					}
				return ret;
				}*/
			
			core::image create_depth_image(const core::manager& manager, const vk::Extent3D& extent) const;

			//vk::UniqueImageView create_depth_image_view(const core::manager& manager, const vk::Image& vk_image) const;

			vk::UniqueFramebuffer create_framebuffer(const core::manager& manager, const window::window& window, size_t image_index) const;
		
			std::vector<vk::UniqueFramebuffer> create_framebuffers(const core::manager& manager, const window::window& window) const;
			
			vk::UniqueBuffer create_buffer(const core::manager& manager, const Model& model, vk::BufferUsageFlags usage_flags, size_t size) const;

			vk::UniqueDeviceMemory create_memory(const core::manager& manager, const vk::Buffer& buffer, vk::MemoryPropertyFlags mem_props_flags) const;

			template <typename T>
			void fill_staging_memory(const core::manager& manager, const Model& model, vk::Buffer& buffer, vk::DeviceMemory& memory, std::vector<T> in_data)
				{
				const auto& device{ manager.getter(this).device() };
				size_t buffer_size{ sizeof(in_data[0]) * in_data.size() };
				
				utils::observer_ptr<void> data{ device.mapMemory(memory, 0, buffer_size, vk::MemoryMapFlags{})};
				
				//std::copy(in_data.begin(), in_data.end(), data);
				memcpy(data, in_data.data(), buffer_size);

				device.unmapMemory(memory);
				}

			void copy_buffer(core::manager& manager, const vk::Buffer& src, vk::Buffer dst, size_t size);

			void record_commands(const window::window& window, vk::CommandBuffer& command_buffer, uint32_t image_index, float delta_time);
		};
	}