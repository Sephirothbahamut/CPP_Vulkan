#pragma once

#include <vulkan/vulkan.hpp>

#include "instance.h"
#include "debug_messenger.h"

namespace utils::graphics::vulkan
	{
	class manager
		{
		public:
			manager() : 
				instance{"name", debug_messenger::create_info()},
				dld{instance.get(), vkGetInstanceProcAddr},
				debug_messenger{instance.get(), dld}
				{
				}

		private:
			instance instance;
			vk::DispatchLoaderDynamic dld;
			debug_messenger debug_messenger;
		};
	}