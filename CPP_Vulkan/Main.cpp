#include <chrono>

#include <utils_win32/transparent.h>

#include "vulkan/core/utils.h"
#include "vulkan/core/manager.h"
#include "vulkan/core/shader.h"
#include "vulkan/window/window.h"
#include "vulkan/window/window_sized_image.h"
#include "vulkan/renderer/rectangle/rectangle_renderer.h"

#include "vulkan/core/model.h"

#include "iige/loop.h"
#include "iige/resources_manager.h"
#include "utils/self_consuming_queue.h"

class vulkan_window : public utils::win32::window::t<utils::graphics::vulkan::window::window>//, utils::win32::window::transparent<utils::win32::window::transparency_t::composition_attribute>
	{
	utils_devirtualize
	public:
		//using t<window_implementation_ts...>::t;
		vulkan_window(base::create_info c_info, utils::graphics::vulkan::core::manager& manager) :
			base{ c_info }, utils::graphics::vulkan::window::window{ manager } {}
	};

struct thing_t
	{
	thing_t(const std::filesystem::path& fname) { throw std::runtime_error{"trolololol"}; }
	};



int main()
	{
	namespace ugv = utils::graphics::vulkan;
	iige::resources::manager<thing_t> rm;
	rm.load_async<thing_t>("hello");

	try
		{
		ugv::core::manager manager;

		//ugv::renderer::rectangle_renderer rect_renderer{manager};
		//Model model = load_model("data/models/spyro/spyro.obj");
		//Model model = load_model("data/models/bunny.obj");

		ugv::renderer::rectangle_renderer rectangle_renderer{ manager };

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

		auto closer{ manager.get_closer() };

		iige::loop::variable_fps_and_game_speed loop;
		loop.step = [&window](float delta_time) -> bool
			{
			while (window.poll_event()) {}
			return window.is_open();
			};
		loop.draw = [&manager, &window, &rectangle_renderer](float delta_time, float interpolation) -> void
			{
			if (window.width & window.height)
				{
				rectangle_renderer.draw(manager, window, delta_time);
				}
			};
		loop.run();
		
		}
	catch (const std::exception& e)
		{
		ugv::core::logger.err(e.what());
		}
	return 0;
	}