#pragma once

#include "../../dependencies.h"

#include "../../core/shader.h"
#include "../../core/renderer.h"
#include "../../core/model.h"
#include <vulkan/vulkan.h>

namespace utils::graphics::vulkan::renderer
	{
	class renderer_3d : public core::renderer
		{
		public:
			renderer_3d(const core::manager& manager, const Model& model);

			void resize(const core::manager& manager, const window::window& window) final override;

			void draw(core::manager& manager, const window::window& window) final override;

		private:
			core::shader_vertex                 vertex_shader         ;
			core::shader_fragment               fragment_shader       ;
			vk::UniqueRenderPass                vk_unique_renderpass  ;
			vk::UniquePipeline                  vk_unique_pipeline    ;
			std::vector<vk::UniqueFramebuffer>  vk_unique_framebuffers;
			vk::UniqueBuffer                    vk_unique_buffer      ;
			vk::UniqueDeviceMemory              vk_unique_memory      ;
			size_t vertices_count;

			vk::UniqueRenderPass create_renderpass  (const core::manager& manager) const;
		
			vk::UniquePipeline   create_pipeline    (const core::manager& manager, const vk::RenderPass& renderpass, const core::shader_vertex& vertex_shader, const core::shader_fragment& fragment_shader) const;
		
			vk::UniqueFramebuffer create_framebuffer(const core::manager& manager, const window::window& window, size_t image_index) const;
		
			std::vector<vk::UniqueFramebuffer> create_framebuffers(const core::manager& manager, const window::window& window) const;
			
			vk::UniqueBuffer create_buffer(const core::manager& manager, const Model& model) const
				{
				vk::BufferCreateInfo bufferInfo
					{
						.size        { sizeof(model.vertices[0]) * model.vertices.size() },
						.usage       { vk::BufferUsageFlagBits::eVertexBuffer },
						.sharingMode { vk::SharingMode::eExclusive },
					};
				return manager.getter(this).device().createBufferUnique(bufferInfo);
				}

			uint32_t find_memory_type(const core::manager& manager, uint32_t type_filter, vk::MemoryPropertyFlags mem_props_flags) const 
				{
				vk::PhysicalDeviceMemoryProperties mem_properties{ manager.getter(this).physical_device().getMemoryProperties() };
				
				for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
					{
					if ((type_filter & (1 << i)) && ((mem_properties.memoryTypes[i].propertyFlags & mem_props_flags) == mem_props_flags))
						{
						return i;
						}
					}

				throw std::runtime_error("failed to find suitable memory type!");
				}

			vk::UniqueDeviceMemory create_memory(const core::manager& manager) const
				{
				const auto& device{ manager.getter(this).device() };
					
				vk::MemoryRequirements mem_requirements{ device.getBufferMemoryRequirements(vk_unique_buffer.get()) };

				vk::MemoryAllocateInfo alloc_info
					{
					.allocationSize{mem_requirements.size},
					.memoryTypeIndex{find_memory_type(manager, mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)}
					};
				return device.allocateMemoryUnique(alloc_info);
				}
				
			void bind_memory(const core::manager& manager)
				{
				const auto& device{ manager.getter(this).device() };
				device.bindBufferMemory(vk_unique_buffer.get(), vk_unique_memory.get(), 0);
				}

			void fill_memory(const core::manager& manager, const Model& model)
				{
				const auto& device{ manager.getter(this).device() };
				size_t buffer_size{ sizeof(model.vertices[0]) * model.vertices.size() };
				
				utils::observer_ptr<void> data{ device.mapMemory(vk_unique_memory.get(), 0, buffer_size, vk::MemoryMapFlags{})};
				
				//std::copy(model.vertices.begin(), model.vertices.end(), data);
				memcpy(data, model.vertices.data(), buffer_size);

				device.unmapMemory(vk_unique_memory.get());
				}

			void record_commands(const window::window& window, vk::CommandBuffer& command_buffer, uint32_t image_index);
		};
	}