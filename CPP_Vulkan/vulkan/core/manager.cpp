#include "manager.h"

#include "utils.h"

namespace utils::graphics::vulkan::core
	{
		manager::manager() :
			instance                         { "name", debug_messenger::create_info() },
			dld                              { instance.get(), vkGetInstanceProcAddr },
#ifdef utils_is_debug		                 
			debug_messenger                  { instance.get(), dld },
#endif						                 
			physical_device                  { instance.get(), /* out */ swapchain_chosen_details},
			device                           { physical_device, physical_device.get_graphics_family_index(), physical_device.get_present_family_index() },
			queues                           { device.get(),    physical_device.get_graphics_family_index(), physical_device.get_present_family_index() },
			flying_frames_pool               { device.get(), queues },
			vk_unique_memory_op_command_pool { create_memory_op_command_pool() }
			{}

		vk::UniqueCommandPool manager::create_memory_op_command_pool()
			{
			try
				{
				return device.get().createCommandPoolUnique
				({
					.flags{vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient},
					.queueFamilyIndex{queues.get_graphics().index},
					});
				}
			catch (vk::SystemError err)
				{
				throw std::runtime_error{ "Failed to create command pool!" };
				}
			}
}