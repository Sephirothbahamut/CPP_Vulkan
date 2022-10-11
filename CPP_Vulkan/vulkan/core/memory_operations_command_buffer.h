#pragma once

#include "manager.h"

namespace utils::graphics::vulkan::core
	{
	class memory_operations_command_buffer
		{
		public:
			memory_operations_command_buffer(utils::graphics::vulkan::core::manager& manager) : manager_ptr{&manager} {}

			void begin()
				{
				const auto& device{manager_ptr->get_device()};

				vk::CommandBufferAllocateInfo alloc_info
					{
					.commandPool { manager_ptr->get_memory_op_command_pool() },
					.level { vk::CommandBufferLevel::ePrimary },
					.commandBufferCount { 1 },
					};

				memory_operations_command_buffer_unique = std::move(device.allocateCommandBuffersUnique(alloc_info)[0]);

				vk::CommandBufferBeginInfo beginInfo
					{
					.flags {vk::CommandBufferUsageFlagBits::eOneTimeSubmit}
					};

				memory_operations_command_buffer_unique->begin(beginInfo);
				}

			void submit()
				{
				const auto& graphics_queue{manager_ptr->get_queues().get_graphics().queue};

				memory_operations_command_buffer_unique->end();

				vk::SubmitInfo submitInfo
					{
					.commandBufferCount { 1 },
					.pCommandBuffers { &memory_operations_command_buffer_unique.get() },
					};


				graphics_queue.submit(1, &submitInfo, VK_NULL_HANDLE);
				graphics_queue.waitIdle();

				memory_operations_command_buffer_unique.reset();
				}

			~memory_operations_command_buffer() { if (memory_operations_command_buffer_unique) { submit(); } }

			vk::CommandBuffer& get() noexcept { return memory_operations_command_buffer_unique.get(); }
			
		private:
			utils::observer_ptr<utils::graphics::vulkan::core::manager> manager_ptr;
			vk::UniqueCommandBuffer memory_operations_command_buffer_unique;
		};
	}