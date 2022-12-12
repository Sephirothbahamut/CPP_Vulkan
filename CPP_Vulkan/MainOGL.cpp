#include <chrono>

#include <utils/win32/window/window.h>
#include <utils/win32/window/regions.h>
#include <utils/win32/window/style.h>

#include <glad.h>

#include "opengl/window.h"
#include "opengl/shader.h"
#include "opengl/rectangle_renderer.h"

#include "iige/loop.h"

struct window_ogl : 
	public utils::win32::window::t
		<
		utils::win32::window::style,
		utils::win32::window::resizable_edge,
		utils::win32::window::regions,
		utils::graphics::opengl::window::window
		>, 
	utils::devirtualize
	{
	struct create_info
		{
		utils::win32::window::base          ::create_info base          ;
		utils::win32::window::style         ::create_info style         ;
		utils::win32::window::resizable_edge::create_info resizable_edge;
		utils::win32::window::regions       ::create_info regions       ;
		};

	window_ogl(create_info create_info) :
		utils::win32::window::base          {create_info.base          },
		utils::win32::window::style         {create_info.style         },
		utils::win32::window::resizable_edge{create_info.resizable_edge},
		utils::win32::window::regions       {create_info.regions       }
		{}
	};

int mainz()
	{
	try
		{
		window_ogl::initializer i;
		window_ogl window{window_ogl::create_info
			{
			.base
				{
				.title{L"Sample Window Class"},
				.size{800, 600}
				},
			.style
				{
				.transparency{utils::win32::window::style::transparency_t::composition_attribute},
				.borders{utils::win32::window::style::value_t::enable},
				.shadow {utils::win32::window::style::value_t::enable}
				}
			}};

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
			if (window.client_rect.width & window.client_rect.height)
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