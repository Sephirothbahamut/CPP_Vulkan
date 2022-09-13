#include <chrono>

#include "vulkan/core/utils.h"
#include "vulkan/core/manager.h"
#include "vulkan/core/shader.h"
#include "vulkan/window/window.h"
#include "vulkan/window/window_sized_image.h"
#include "vulkan/renderer/rectangle/rectangle_renderer.h"

#include "vulkan/core/model.h"

#include <utils_win32/transparent.h>
class vulkan_window : public utils::win32::window::t<utils::graphics::vulkan::window::window>//, utils::win32::window::transparent<utils::win32::window::transparency_t::composition_attribute>
	{
	utils_devirtualize
	public:
		//using t<window_implementation_ts...>::t;
		vulkan_window(base::create_info c_info, utils::graphics::vulkan::core::manager& manager) :
			base{ c_info }, utils::graphics::vulkan::window::window{ manager } {}
	};

int main()
	{
	namespace ugv = utils::graphics::vulkan;
	try
		{
		ugv::core::manager manager;

		//ugv::renderer::rectangle_renderer rect_renderer{manager};
		//Model model = load_model("data/models/spyro/spyro.obj");
		//Model model = load_model("data/models/bunny.obj");
	
		ugv::renderer::rectangle_renderer rectangle_renderer{manager};

		vulkan_window::initializer i;
		vulkan_window window
			{
			vulkan_window::create_info
				{
				.title{L"Sample Window Class"},
				.size{800, 600}
				},
			manager
			};
	
		
		manager.bind(window, rectangle_renderer);
		window.renderer_ptr = &rectangle_renderer;
		//window.width = 601;

		auto closer{manager.get_closer()};

		std::chrono::time_point prev_step_time { std::chrono::high_resolution_clock::now() };
		std::chrono::time_point curr_step_time { std::chrono::high_resolution_clock::now() };

		uint32_t frames_counter{ 0 };

		while (window.is_open())
			{
			prev_step_time = curr_step_time;
			curr_step_time = std::chrono::high_resolution_clock::now();

			std::chrono::duration<float> step_delta_time{ curr_step_time - prev_step_time };
			while (window.poll_event())
				{
				}
			if (window.is_open())
				{
				if (window.width & window.height)
					{
					rectangle_renderer.draw(manager, window, step_delta_time.count());
					}
				}
			}
		}
	catch (const std::exception& e)
		{
		ugv::core::logger.err(e.what());
		}
	return 0;
	}