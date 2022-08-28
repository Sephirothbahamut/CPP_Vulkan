#include "queues.h"

#include "../dependencies.h"

namespace utils::graphics::vulkan::core
	{
	queues::queues(const vk::Device& device, uint32_t graphics_family_index, uint32_t present_family_index) :
		graphics{device.getQueue(graphics_family_index, 0), graphics_family_index},
		present {device.getQueue(present_family_index,  0), present_family_index }
		{}
	}
