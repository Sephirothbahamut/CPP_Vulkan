#pragma once

#include "../../window/window.h"
#include "../../core/model.h"
#include "../../resources/shader.h"
#include "../../resources/vertex.h"

namespace utils::graphics::vulkan::renderer
	{
	class rectangle_renderpass
		{

		public:
			rectangle_renderpass(core::manager& manager);

			vk::RenderPass get_renderpass() { return vk_unique_renderpass.get(); }


			void record_commands(const window::window& window, vk::CommandBuffer& command_buffer, vk::Framebuffer framebuffer, float delta_time);

		private:
			vk::UniqueRenderPass              vk_unique_renderpass;
			core::shader_vertex               vertex_shader;
			core::shader_fragment             fragment_shader;
			vk::UniquePipelineLayout          vk_unique_pipeline_layout;
			vk::UniquePipeline                vk_unique_pipeline;

			std::array<utils::math::vec2f, 6> vertices
				{{
					{0.0f, 0.0f},
					{1.0f, 0.0f},
					{0.0f, 1.0f},
					{1.0f, 0.0f},
					{1.0f, 1.0f},
					{0.0f, 1.0f}
				}};

			vk::UniqueBuffer                  vk_unique_staging_vertex_buffer;
			vk::UniqueDeviceMemory            vk_unique_staging_vertex_memory;
			vk::UniqueBuffer                  vk_unique_vertex_buffer;
			vk::UniqueDeviceMemory            vk_unique_vertex_memory;

			vk::UniqueRenderPass     create_renderpass(const core::manager& manager) const;

			vk::UniquePipelineLayout create_pipeline_layout(const core::manager& manager) const;

			vk::UniquePipeline       create_pipeline(const core::manager& manager, const vk::RenderPass& renderpass, const core::shader_vertex& vertex_shader, const core::shader_fragment& fragment_shader) const;

			vk::UniqueBuffer         create_buffer(const core::manager& manager, vk::BufferUsageFlags usage_flags, size_t size) const;

			vk::UniqueDeviceMemory   create_memory(const core::manager& manager, const vk::Buffer& buffer, vk::MemoryPropertyFlags mem_props_flags) const;

			template <typename T>
			void fill_staging_memory(const core::manager& manager, vk::Buffer& buffer, vk::DeviceMemory& memory, const std::array<T,6>& in_data)
				{
				const auto& device{ manager.get_device() };
				size_t buffer_size{ sizeof(in_data[0]) * in_data.size() };

				utils::observer_ptr<void> data{ device.mapMemory(memory, 0, buffer_size, vk::MemoryMapFlags{}) };

				//std::copy(in_data.begin(), in_data.end(), data);
				memcpy(data, in_data.data(), buffer_size);

				device.unmapMemory(memory);
				}

			void copy_buffer(core::manager& manager, const vk::Buffer& src, vk::Buffer dst, size_t size);
		};
	}