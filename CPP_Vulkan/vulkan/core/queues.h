#pragma once

#include <vulkan/vulkan.hpp>

#include <optional>

namespace utils::graphics::vulkan::core
	{
	class queues
		{
		public:
			queues(const vk::Device& device, uint32_t graphics_family_index, uint32_t present_family_index);

			struct queue_t { vk::Queue queue;  uint32_t index; }; 

			queue_t get_graphics() const noexcept { return graphics; }
			queue_t get_present () const noexcept { return present;  }

		private:
			queue_t graphics; 
			queue_t present;

		};
	}