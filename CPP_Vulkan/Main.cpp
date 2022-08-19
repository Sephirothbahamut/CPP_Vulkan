#include "vulkan/core/manager.h"
#include "vulkan/window/window.h"

class vulkan_window : public utils::win32::window::t<utils::graphics::vulkan::window::window>
	{
	utils_devirtualize
	public:
		//using t<window_implementation_ts...>::t;
		vulkan_window(base::create_info c_info, const utils::graphics::vulkan::core::manager& manager) : base{ c_info }, utils::graphics::vulkan::window::window{ manager } {}
	};

int main()
	{
	namespace ugv = utils::graphics::vulkan;
	ugv::core::manager manager;

	vulkan_window::initializer i;
	vulkan_window window
		{ vulkan_window::create_info
			{
			.title{L"Sample Window Class"},
			},
		manager
		};

	while (window.is_open())
		{
		while (window.poll_event())
			{
			}
		}
	}