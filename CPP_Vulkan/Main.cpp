#include <chrono>

#include <utils/win32/window/window.h>
#include <utils/win32/window/regions.h>
#include <utils/win32/window/style.h>

#include <utils/logger.h>

#include "vulkan/core/utils.h"
#include "vulkan/core/manager.h"
#include "vulkan/resources/shader.h"
#include "vulkan/window/window.h"
#include "vulkan/window/window_sized_image.h"
#include "vulkan/renderer/rectangle/rectangle_renderpass.h"

#include "vulkan/core/model.h"

#include "iige/loop.h"
#include "iige/assets_manager.h"

struct window : 
	public utils::win32::window::t
		<
		utils::win32::window::style,
		utils::win32::window::resizable_edge,
		utils::win32::window::regions,
		utils::graphics::vulkan::window::window
		>, 
	utils::devirtualize
	{
	struct create_info
		{
		utils::win32::window::base          ::create_info base          ;
		utils::win32::window::style         ::create_info style         ;
		utils::win32::window::resizable_edge::create_info resizable_edge;
		utils::win32::window::regions       ::create_info regions       ;
		utils::graphics::vulkan::core::manager&           vulkan_manager;
		};

	window(create_info create_info) : 
		utils::win32::window::base             {create_info.base          },
		utils::win32::window::style            {create_info.style         },
		utils::win32::window::resizable_edge   {create_info.resizable_edge},
		utils::win32::window::regions          {create_info.regions       },
		utils::graphics::vulkan::window::window{create_info.vulkan_manager}
		{
		utils::graphics::vulkan::window::window::recreate_swapchain();
		auto& swapchain{utils::graphics::vulkan::window::window::get_swapchain()};
		auto window_extent{get_extent()};
		auto swapchain_size{swapchain.size()};
		}
	};


int main()
	{
	namespace ugv = utils::graphics::vulkan;
	
	try
		{
		ugv::core::manager manager;
	
		window::initializer i;
		window window{window::create_info
			{
			.base
				{
				.title{L"Sample Window Class"},
				.size{1024, 600}
				},
			.style
				{
				.transparency{utils::win32::window::style::transparency_t::composition_attribute},
				.borders{utils::win32::window::style::value_t::enable}
				},
			.regions
				{
				.default_hit_type{utils::win32::window::hit_type::drag}
				},
			.vulkan_manager{manager}
			}};

		ugv::renderer::rectangle_renderpass tmp_renderpass{manager};

		ugv::core::renderer renderer {manager, window , tmp_renderpass};

		auto closer{ manager.get_closer() };
	
		iige::loop::variable_fps_and_game_speed loop;


		auto draw_callback = [&manager, &window, &renderer, &tmp_renderpass](float delta_time, float interpolation) -> void
			{
			if (window.client_rect.width && window.client_rect.height)
				{
				auto frame{renderer.begin_frame(delta_time)};

				frame.tmp_record_commands(delta_time);
				}
			};

		window.resize_redraw_callback = [&window, &loop, &draw_callback]() { draw_callback(loop.update_delta_time(), loop.update_interpolation()); };

		loop.step = [&window](float delta_time) -> bool
			{
			while (window.poll_event()) {}
			return window.is_open();
			};
		loop.draw = draw_callback;

		loop.run();
		}
	catch (const std::exception& e)
		{
		ugv::core::logger.err(e.what());
		}
	return 0;
	}