#pragma once
#include "../dependencies.h"

#include "../core/queues.h"

namespace utils::graphics::vulkan::core
	{
	struct flying_frame
		{
		vk::CommandBuffer   vk_command_buffer;
		vk::Semaphore       vk_semaphore_image_available;
		vk::Semaphore       vk_semaphore_render_finished;
		vk::Fence           vk_fence_frame_in_flight;
		};

	class flying_frames_pool
		{
		public:
			flying_frames_pool(const vk::Device& device, const queues& queues);

			flying_frame get() noexcept;

		private:
			struct unique_flying_frame
				{
				vk::UniqueCommandBuffer   vk_command_buffer;
				vk::UniqueSemaphore       vk_semaphore_image_available;
				vk::UniqueSemaphore       vk_semaphore_render_finished;
				vk::UniqueFence           vk_fence_frame_in_flight;
				};
			
			vk::UniqueCommandPool vk_command_pool;
			std::vector<unique_flying_frame> flying_frames;
			size_t current_index;

			vk::UniqueCommandPool create_command_pool(const vk::Device& device, const queues& queues);

			std::vector<unique_flying_frame> create_flying_frames(const vk::Device& device);
		};
	}
