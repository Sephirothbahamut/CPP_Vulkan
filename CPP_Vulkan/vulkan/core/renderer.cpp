#include "renderer.h"

#include "../window/window.h"

namespace utils::graphics::vulkan::core
	{
	renderer::renderer(vk::UniqueRenderPass&& vk_unique_renderpass) : vk_unique_renderpass{ std::move(vk_unique_renderpass) } {}
	
	renderer::~renderer()
		{
		for (auto window_dependent_data_ptr : window_dependent_data_ptrs)
			{
			delete window_dependent_data_ptr.second;
			}
		}
	
	// getter_renderer_window_data_const

	renderer::getter_renderer_window_data_const::getter_renderer_window_data_const(const renderer& renderer) : renderer_ptr{ &renderer } {}

	const std::vector<core::image::create_info>& renderer::getter_renderer_window_data_const::window_sized_images_create_info() const noexcept
		{
		return renderer_ptr->window_sized_images_create_info;
		}
	const std::vector<vk::ImageView>& renderer::getter_renderer_window_data_const::image_views() const noexcept
		{
		return renderer_ptr->image_views;
		}

	const vk::RenderPass& renderer::getter_renderer_window_data_const::renderpass() const noexcept
		{
		return renderer_ptr->vk_unique_renderpass.get();
		}

	// getter_renderer_window_data

	renderer::getter_renderer_window_data::getter_renderer_window_data(renderer& renderer) : renderer_ptr{ &renderer } {}

	const std::vector<core::image::create_info>& renderer::getter_renderer_window_data::window_sized_images_create_info() const noexcept
		{
		return renderer_ptr->window_sized_images_create_info;
		}
	const std::vector<vk::ImageView>& renderer::getter_renderer_window_data::image_views() const noexcept
		{
		return renderer_ptr->image_views;
		}
	const vk::RenderPass& renderer::getter_renderer_window_data::renderpass() const noexcept
		{
		return renderer_ptr->vk_unique_renderpass.get();
		}
	std::unordered_map<observer_ptr<const window::window>, observer_ptr<renderer_window_data>>& renderer::getter_renderer_window_data::window_dependent_data_ptrs() noexcept
		{
		return renderer_ptr->window_dependent_data_ptrs;
		}

	}