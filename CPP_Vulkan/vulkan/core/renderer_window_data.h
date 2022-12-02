//#pragma once
//
//#include <vector>
//
//#include <utils/memory.h>
//
//#include "manager.h"
//
//namespace utils::graphics::vulkan::core   { class renderer; }
//namespace utils::graphics::vulkan::window { class window;   }
//
//namespace utils::graphics::vulkan::core
//	{
//	class renderer_window_data
//		{
//		public:
//			renderer_window_data(const manager& manager, renderer& renderer, window::window& window);
//
//			// Getters
//			class getter_window
//				{
//				friend class renderer_window_data;
//				friend class window::window;
//
//				getter_window(renderer_window_data& renderer_window_data);
//
//				void resize(const manager& manager, const window::window& window);
//
//				utils::observer_ptr<renderer_window_data> renderer_window_data_ptr;
//				};
//			class getter_renderer
//				{
//				friend class renderer_window_data;
//				friend class core::renderer;
//
//				getter_renderer(const renderer_window_data& renderer_window_data);
//
//				public:
//					const vk::Framebuffer& framebuffer(size_t index)   const noexcept;
//					const std::vector<vk::Framebuffer> framebuffers() const noexcept;
//
//				private:
//					utils::observer_ptr<const renderer_window_data> renderer_window_data_ptr;
//				};
//
//			friend class getter_window;
//			friend class getter_renderer;
//
//			inline getter_window   getter(const window::window*) noexcept { return { *this }; }
//			inline getter_renderer getter(const core::renderer*) noexcept { return { *this }; }
//			
//		private:
//			utils::observer_ptr<renderer> renderer_ptr;
//			utils::observer_ptr<window::window> window_ptr;
//
//			std::vector<core::image> window_sized_images;
//			std::vector<vk::UniqueFramebuffer> framebuffers;
//
//			vk::UniqueFramebuffer create_framebuffer(const core::manager& manager, const window::window& window, size_t image_index) const;
//
//			std::vector<vk::UniqueFramebuffer> create_framebuffers(const core::manager& manager, const window::window& window) const;
//
//			void resize(const manager& manager, const window::window& window);
//		};
//	}