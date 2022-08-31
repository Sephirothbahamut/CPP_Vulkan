#include "flying_frames_pool.h"

namespace utils::graphics::vulkan::core
	{
	flying_frames_pool::flying_frames_pool(const vk::Device& device, const queues& queues) :
				vk_command_pool{ create_command_pool (device, queues) },
				flying_frames  { create_flying_frames(device) },
				current_index  { flying_frames.size() - 1 } {}

	flying_frame flying_frames_pool::get() noexcept
		{
		current_index = (current_index + 1) % flying_frames.size(); //pre-increments, starting from last index the first time
		auto& current{ flying_frames[current_index] };
		return flying_frame
			{
				.vk_command_buffer           {current.vk_command_buffer           .get()},
				.vk_semaphore_image_available{current.vk_semaphore_image_available.get()},
				.vk_semaphore_render_finished{current.vk_semaphore_render_finished.get()},
				.vk_fence_frame_in_flight    {current.vk_fence_frame_in_flight    .get()},
			};
		}

	vk::UniqueCommandPool flying_frames_pool::create_command_pool(const vk::Device& device, const queues& queues)
		{
		vk::UniqueCommandPool ret;
		try
			{
			ret = device.createCommandPoolUnique
			({
				.flags{vk::CommandPoolCreateFlagBits::eResetCommandBuffer},
				.queueFamilyIndex{queues.get_graphics().index},
				});
			}
		catch (vk::SystemError err)
			{
			throw std::runtime_error{ "Failed to create command pool!" };
			}
		return ret;
		}

	std::vector<flying_frames_pool::unique_flying_frame> flying_frames_pool::create_flying_frames(const vk::Device& device)
		{
		std::vector<unique_flying_frame> ret;
		auto command_buffers = device.allocateCommandBuffersUnique(vk::CommandBufferAllocateInfo
			{
				.commandPool { vk_command_pool.get() },
				.level { vk::CommandBufferLevel::ePrimary },
				.commandBufferCount { 2 }, // max frames in flight amount recommended
			}); //TODO we'll discover why we need a vector of these

		for (size_t i = 0; i < command_buffers.size(); i++)
			{
			auto& command_buffer = command_buffers[i];
			try
				{
				auto image_available{ device.createSemaphoreUnique({}) };
				auto render_finished{ device.createSemaphoreUnique({}) };
				auto frame_in_flight{ device.createFenceUnique({.flags{vk::FenceCreateFlagBits::eSignaled}}) };
				ret.emplace_back(std::move(command_buffer), std::move(image_available), std::move(render_finished), std::move(frame_in_flight));
				}
			catch (vk::SystemError system_error) { throw std::runtime_error{ "Failed to create semaphores/fence" }; }

			}

		return ret;
		}
	}
