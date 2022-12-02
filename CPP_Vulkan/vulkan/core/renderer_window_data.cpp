//#include "renderer_window_data.h"
//
//#include "renderer.h"
//#include "../window/window.h"
//
//namespace utils::graphics::vulkan::core
//	{
//
//	renderer_window_data::~renderer_window_data()
//		{
//		// remove self from window's vector of observer pointers
//		auto& window_vec{ window_ptr->getter(this).renderer_dependent_data_ptrs() };
//		window_vec.erase(std::remove(window_vec.begin(), window_vec.end(), this), window_vec.end());
//
//		// remove self from renderer's vector of observer pointers
//		auto& renderer_map{ renderer_ptr->window_dependent_data_ptrs };
//		renderer_map.erase(renderer_map.find(this->window_ptr));
//		}
//
//	// getter_window
//
//	renderer_window_data::getter_window::getter_window(renderer_window_data& renderer_window_data) : renderer_window_data_ptr{ &renderer_window_data } {}
//
//	void renderer_window_data::getter_window::resize(const manager& manager, const window::window& window)
//		{
//		renderer_window_data_ptr->resize(manager, window);
//		}
//
//	//getter_renderer
//
//	renderer_window_data::getter_renderer::getter_renderer(const renderer_window_data& renderer_window_data) : renderer_window_data_ptr{ &renderer_window_data } {}
//
//	const vk::Framebuffer& renderer_window_data::getter_renderer::framebuffer(size_t index) const noexcept
//		{
//		return renderer_window_data_ptr->framebuffers[index].get();
//		}
//
//	const std::vector<vk::Framebuffer> renderer_window_data::getter_renderer::framebuffers() const noexcept
//		{
//		std::vector<vk::Framebuffer> ret;
//		for (const auto& e : renderer_window_data_ptr->framebuffers) { ret.push_back(e.get()); }
//		return ret;
//		}
//
//	renderer_window_data::renderer_window_data(const manager& manager, renderer& renderer, window::window& window) :
//		renderer_ptr{&renderer},
//		window_ptr{&window},
//		framebuffers{create_framebuffers(manager, window)} {}
//
//	vk::UniqueFramebuffer renderer_window_data::create_framebuffer(const core::manager& manager, const window::window& window, size_t image_index) const
//		{
//		try
//			{
//			// concatenation of swapchain imageview + window sized imageviews + renderer imageviews
//			auto& renderer_image_views = renderer_ptr->image_views;
//			size_t attachments_amount = 1 + window_sized_images.size() + renderer_image_views.size();
//
//			std::vector<vk::ImageView> attachments{ window.get_swapchain().get_image_view(image_index) };
//			attachments.reserve(attachments_amount);
//			for (const auto& ws_image : window_sized_images) { attachments.push_back(ws_image.view()); }
//			attachments.insert(attachments.end(), renderer_image_views.begin(), renderer_image_views.end());
//
//			return 
//				{ manager.get_device().createFramebufferUnique(
//					vk::FramebufferCreateInfo
//					{
//						.renderPass      { renderer_ptr->vk_unique_renderpass.get() },
//						.attachmentCount { static_cast<uint32_t>(attachments.size()) },
//						.pAttachments    { attachments.data() },
//						.width           { window.width },
//						.height          { window.height },
//						.layers          { 1 },
//					}
//				) };
//			}
//		catch (vk::SystemError err)
//			{
//			throw std::runtime_error{ "Failed to create framebuffer!" };
//			}
//		}
//
//	std::vector<vk::UniqueFramebuffer> renderer_window_data::create_framebuffers(const core::manager& manager, const window::window& window) const
//		{
//		auto& swapchain{ window.get_swapchain() };
//		std::vector<vk::UniqueFramebuffer> ret;
//		ret.reserve(swapchain.size());
//
//		for (size_t i = 0; i < swapchain.size(); i++)
//			{
//			auto framebuffer{ create_framebuffer(manager, window, i) };
//			ret.push_back(std::move(framebuffer));
//			}
//
//		return ret;
//		}
//
//	void renderer_window_data::resize(const manager& manager, const window::window& window)
//		{
//		window_sized_images.clear();
//		for (auto& image_create_info : renderer_ptr->window_sized_images_create_info)
//			{
//			window_sized_images.emplace_back(manager, image_create_info, window.extent);
//			}
//		framebuffers = create_framebuffers(manager, window);
//		}
//}

