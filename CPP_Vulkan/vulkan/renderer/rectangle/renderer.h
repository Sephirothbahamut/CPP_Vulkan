#pragma once

#include "../../dependencies.h"

#include "../../core/shader.h"
#include "../../core/renderer.h"

namespace utils::graphics::vulkan::renderer
	{
	class rectangle_renderer : public core::renderer
		{
		public:
			rectangle_renderer(const core::manager& manager);

			void resize(const core::manager& manager, const window::window& window) final override;

			void draw(core::manager& manager, const window::window& window, float delta_time) final override;

		private:
			core::shader_vertex                 vertex_shader  ;
			core::shader_fragment               fragment_shader;
			vk::UniqueRenderPass                vk_renderpass  ;
			vk::UniquePipeline                  vk_pipeline    ;
			std::vector<vk::UniqueFramebuffer>  vk_framebuffers;

			vk::UniqueRenderPass create_renderpass(const core::manager& manager) const;
		
			vk::UniquePipeline create_pipeline(const core::manager& manager, const vk::RenderPass& renderpass, const core::shader_vertex& vertex_shader, const core::shader_fragment& fragment_shader) const;
		
			vk::UniqueFramebuffer create_framebuffer(const core::manager& manager, const window::window& window, size_t image_index) const;
		
			std::vector<vk::UniqueFramebuffer> create_framebuffers(const core::manager& manager, const window::window& window) const;

			void record_commands(const window::window& window, vk::CommandBuffer& command_buffer, uint32_t image_index);
		};
	}