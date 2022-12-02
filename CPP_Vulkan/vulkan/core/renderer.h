#pragma once

#include <vector>

#include "../window/window.h"
#include "image.h"
#include "../resources/shader.h"

#include "../renderer/rectangle/rectangle_renderpass.h"

#include "renderer_window_data.h"

namespace utils::graphics::vulkan::renderer { class rectangle_renderpass; }

namespace utils::graphics::vulkan::core
	{
	class frame;
	class renderer
		{
		friend class window::window;
		friend class frame;

		public:
			//TODO deal with tmp
			renderer(core::manager& manager, window::window& window, utils::graphics::vulkan::renderer::rectangle_renderpass& tmp) : manager_ptr{&manager}, window_ptr{&window}, tmp_ptr{&tmp}, framebuffers { create_framebuffers() } {}

			[[nodiscard]] frame begin_frame(float delta_time);

		protected:
			utils::observer_ptr<core::manager > manager_ptr;
			utils::observer_ptr<window::window> window_ptr;

			std::vector<core::image::create_info> window_sized_images_create_info;
			std::vector<vk::ImageView> image_views;
			utils::observer_ptr<utils::graphics::vulkan::renderer::rectangle_renderpass> tmp_ptr;

			std::vector<core::image> window_sized_images;
			std::vector<vk::UniqueFramebuffer> framebuffers;

			vk::UniqueFramebuffer create_framebuffer(size_t image_index) const
				{
				try
					{
					// concatenation of swapchain imageview + window sized imageviews + renderer imageviews
					auto& renderer_image_views = image_views;
					size_t attachments_amount = 1 + window_sized_images.size() + renderer_image_views.size();

					std::vector<vk::ImageView> attachments{window_ptr->get_swapchain().get_image_view(image_index)};
					attachments.reserve(attachments_amount);
					for (const auto& ws_image : window_sized_images) { attachments.push_back(ws_image.view()); }
					attachments.insert(attachments.end(), renderer_image_views.begin(), renderer_image_views.end());

					return
						{manager_ptr->get_device().createFramebufferUnique(
							vk::FramebufferCreateInfo
							{
								.renderPass      { tmp_ptr->get_renderpass() },
								.attachmentCount { static_cast<uint32_t>(attachments.size()) },
								.pAttachments    { attachments.data() },
								.width           { window_ptr->width },
								.height          { window_ptr->height },
								.layers          { 1 },
							}
						)};
					}
				catch (vk::SystemError err)
					{
					throw std::runtime_error{"Failed to create framebuffer!"};
					}
				}

			std::vector<vk::UniqueFramebuffer> create_framebuffers() const
				{
				auto& swapchain{window_ptr->get_swapchain()};
				std::vector<vk::UniqueFramebuffer> ret;
				ret.reserve(swapchain.size());

				for (size_t i = 0; i < swapchain.size(); i++)
					{
					auto framebuffer{create_framebuffer(i)};
					ret.push_back(std::move(framebuffer));
					}

				return ret;
				}

			void resize()
				{
				window_sized_images.clear();
				for (auto& image_create_info : window_sized_images_create_info)
					{
					window_sized_images.emplace_back(*manager_ptr, image_create_info, window_ptr->extent);
					}
				framebuffers = create_framebuffers();
				}
			

		private:

		};

	class frame
		{
		friend class renderer;

		public:
			frame(const frame& copy) = delete;
			frame(frame&& move) = delete;
			frame& operator=(const frame& copy) noexcept = default;
			frame& operator=(frame&& move) noexcept = default;


			void tmp_record_commands(float delta_time)
				{
				renderer_ptr->tmp_ptr->record_commands(*window_ptr, current_flying_frame.vk_command_buffer, renderer_ptr->framebuffers[image_index].get(), delta_time);
				}

		private:
			frame(core::manager& manager, core::renderer& renderer, window::window& window, float delta_time) :
				manager_ptr{&manager}, renderer_ptr{&renderer}, window_ptr{&window}
				{
				if constexpr (utils::compilation::debug)
					{
					//TODO check error
					//if (vk_unique_framebuffers.empty()) { throw std::runtime_error{ "You forgot to call resize on the current window." }; }
					}

				current_flying_frame = manager.get_flying_frames_pool().get();
				auto& device{manager.get_device()};
				auto& swapchain{window.get_swapchain()};

				if (device.waitForFences(1, &current_flying_frame.vk_fence_frame_in_flight, VK_TRUE, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess)
					{
					throw std::runtime_error{"Failed wait for fences"};
					}

				image_index = swapchain.next_image(manager, current_flying_frame.vk_semaphore_image_available);
				current_flying_frame.vk_command_buffer.reset();

				vk::CommandBufferBeginInfo beginInfo; // for now empty is okay (we dont need primary/secondary buffer info or flags)
				try { current_flying_frame.vk_command_buffer.begin(beginInfo); }
				catch (vk::SystemError err)
					{
					throw std::runtime_error("failed to begin recording command buffer!");
					}
				}

		public:
			~frame()
				{
				auto& device{manager_ptr->get_device()};
				auto& swapchain{window_ptr->get_swapchain()};

				vk::Semaphore          wait_semaphores[]{current_flying_frame.vk_semaphore_image_available};
				vk::Semaphore          signal_semaphores[]{current_flying_frame.vk_semaphore_render_finished};
				vk::PipelineStageFlags wait_stages[]{vk::PipelineStageFlagBits::eColorAttachmentOutput};

				if (device.resetFences(1, &current_flying_frame.vk_fence_frame_in_flight) != vk::Result::eSuccess)
					{
					throw std::runtime_error{"Failed reset fences"};
					}

				try
					{
					manager_ptr->get_queues().get_graphics().queue.submit
					({{
						.waitSemaphoreCount   { 1 },
						.pWaitSemaphores      { wait_semaphores },
						.pWaitDstStageMask    { wait_stages },
						.commandBufferCount   { 1 },
						.pCommandBuffers      { &current_flying_frame.vk_command_buffer },
						.signalSemaphoreCount { 1 },
						.pSignalSemaphores    { signal_semaphores },
					}}, current_flying_frame.vk_fence_frame_in_flight);
					}
				catch (vk::SystemError system_error) { throw std::runtime_error{"Failed to submit to queue"}; }

				swapchain.present(*manager_ptr, current_flying_frame.vk_semaphore_render_finished, image_index);
				}

		private:

			utils::observer_ptr<core::manager> manager_ptr;
			utils::observer_ptr<core::renderer> renderer_ptr;
			utils::observer_ptr<window::window> window_ptr;

			utils::graphics::vulkan::core::flying_frame current_flying_frame;
			uint32_t image_index;
		};

	inline [[nodiscard]] frame renderer::begin_frame(float delta_time) { return frame{*manager_ptr, *this, *window_ptr, delta_time}; }
	}

