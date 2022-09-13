#pragma once

#include <vector>

#include "image.h"
#include "shader.h"

#include "renderer_window_data.h"

namespace utils::graphics::vulkan::core
	{
	class renderer
		{
		friend void manager::bind(window::window& window, core::renderer& renderer);
		public:
			renderer(vk::UniqueRenderPass&& vk_unique_renderpass);

			~renderer();
			
			virtual void draw(core::manager& manager, const window::window& window, float delta_time) = 0;

			// Getters
			class getter_renderer_window_data_const
				{
				friend class renderer_window_data;
				friend class renderer;

				getter_renderer_window_data_const(const renderer& renderer);

				const std::vector<core::image::create_info>& window_sized_images_create_info() const noexcept;
				const std::vector<vk::ImageView>& image_views() const noexcept;
				const vk::RenderPass& renderpass() const noexcept;

				utils::observer_ptr<const renderer> renderer_ptr;
				};
			class getter_renderer_window_data
				{
				friend class renderer_window_data;
				friend class renderer;

				getter_renderer_window_data(renderer& renderer);

				const std::vector<core::image::create_info>& window_sized_images_create_info() const noexcept;
				const std::vector<vk::ImageView>& image_views() const noexcept;
				const vk::RenderPass& renderpass() const noexcept;
				std::unordered_map<observer_ptr<const window::window>, observer_ptr<renderer_window_data>>& window_dependent_data_ptrs() noexcept;


				utils::observer_ptr<renderer> renderer_ptr;
				};

			friend class getter_renderer_window_data_const;
			friend class getter_renderer_window_data;

			inline getter_renderer_window_data_const getter(const core::renderer_window_data*) const noexcept { return { *this }; }
			inline getter_renderer_window_data       getter(      core::renderer_window_data*)       noexcept { return { *this }; }

		protected:
			std::vector<core::image::create_info> window_sized_images_create_info;
			//std::vector<observer_ptr<renderer_window_data>> window_dependent_data_ptrs;
			// TODO flatmap-ify
			std::unordered_map<observer_ptr<const window::window>, observer_ptr<renderer_window_data>> window_dependent_data_ptrs;
			std::vector<vk::ImageView> image_views;
			vk::UniqueRenderPass       vk_unique_renderpass;
			

		private:

		};
	}
