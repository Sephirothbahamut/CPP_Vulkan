#include <chrono>

#include <glad.h>

#include "opengl/window.h"
#include "opengl/shader.h"
#include "opengl/rectangle_renderer.h"

#include "iige/loop.h"

#include <utils_win32/transparent.h>
class opengl_window : public utils::win32::window::t<utils::graphics::opengl::window::window>//, utils::win32::window::transparent<utils::win32::window::transparency_t::composition_attribute>
	{
	utils_devirtualize
	public:
		//using t<window_implementation_ts...>::t;
		opengl_window(base::create_info c_info) :
			base{ c_info } {}
	};

int mainz()
	{
	try
		{

		opengl_window::initializer i;
		opengl_window window
			{
			opengl_window::create_info
				{
				.title{L"Sample Window Class"},
				.size{800, 600}
				}
			};

		if (!gladLoadGL())
			{
			throw std::runtime_error{ "Failed to initialize OpenGL context" };
			}

		utils::graphics::opengl::rectangle_renderer rectangle_renderer;
		utils::graphics::opengl::Shader shader{ "data/shaders/rectangle/oglshader.vert", "data/shaders/rectangle/oglshader.frag", {}, 4, 5 };
		shader.use();

		iige::loop::variable_fps_and_game_speed loop;
		loop.step = [&window](float delta_time) -> bool
			{
			while (window.poll_event()) {}
			return window.is_open();
			};
		loop.draw = [&window, &rectangle_renderer](float delta_time, float interpolation) -> void
			{
			glClear(GL_COLOR_BUFFER_BIT);
			if (window.width & window.height)
				{
				rectangle_renderer.draw();
				}
			window.swap_buffers();
			};
		loop.run();
		}
	catch (const std::exception& e)
		{
		utils::globals::logger.err(e.what());
		}
	return 0;
	}