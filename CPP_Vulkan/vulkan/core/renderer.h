#pragma once

#include "manager.h"
#include "../window/window.h"

namespace utils::graphics::vulkan::core
	{
	class renderer
		{
		public:

			virtual void resize(const core::manager& manager, const window::window& window) = 0;

			virtual void draw(core::manager& manager, const window::window& window) = 0;

		private:
		
		};
	}