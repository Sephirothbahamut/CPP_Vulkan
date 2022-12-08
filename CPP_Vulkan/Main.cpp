#include <chrono>

#include <utils_win32/transparent.h>
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
	struct create_info { int a{ 5 }; };
	thing_t(const create_info& cinfo) : a{cinfo.a}
		{
		}
	int a;
	};
struct thong_t
	{
	thong_t(int a, const std::string& f) : a{ a }, f{ f } 
		{
		if (a == 0) { throw std::runtime_error{"hello"}; }
		if (a == 1) {} std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	int a;
	std::string f;
	};

void test_asset_managers()
	{
	using namespace std::string_literals;

	utils::thread_pool tp;
	iige::one_time_tasks ott;

	//iige::asset::manager_async<int> rm_int{ []() {return int{0}; } };
	iige::asset::manager<thong_t> rm_thong{ tp, ott, []() {return thong_t{9, "default"}; } };
	iige::asset::manager<thing_t> rm_thing{ tp, ott, []() {return thing_t{{}}; } };
	iige::assets_manager rm{ rm_thong, rm_thing };

	

	auto def{ rm_thong.load_async("sdbsdvdssdvd") };
	auto dez{ rm_thong.load_async("pippo", []() {return thong_t{13, "pippo"}; }) };
	auto dex{ rm_thong.load_async("pippo") };

	//auto scd{ rm_thong.get_containing_type().load_sync("wsdhajhsd") };
	auto scd{ rm_thong.load_sync("scd", []() {return thong_t{0, "scd"}; }) };
	auto qwe{ rm_thong.load_async("qwe", []() {return thong_t{3, "qwe"}; }) };

	auto asy{ rm_thong.load_async("asy", []() {return thong_t{1, "asy"}; }) };

	utils::globals::logger.log("Printing default");
	utils::globals::logger.log("def: " + std::to_string(def->a) + ", "s + def->f);
	utils::globals::logger.log("dez: " + std::to_string(dez->a) + ", "s + dez->f);
	utils::globals::logger.log("dex: " + std::to_string(dex->a) + ", "s + dex->f);
	utils::globals::logger.log("scd: " + std::to_string(scd->a) + ", "s + scd->f);
	utils::globals::logger.log("qwe: " + std::to_string(qwe->a) + ", "s + qwe->f);
	utils::globals::logger.log("asy: " + std::to_string(asy->a) + ", "s + asy->f);


	utils::globals::logger.log("Unloading pippo");
	rm_thong.unload_sync("pippo");

	//utils::globals::logger.log("dex u: " + std::to_string(dex->a) + ", "s + dex->f);

	utils::globals::logger.log("Reloading pippo");
	rm_thong.load_sync("pippo");

	utils::globals::logger.log("dex r: " + std::to_string(dex->a) + ", "s + dex->f);
	utils::globals::logger.log("dez r: " + std::to_string(dez->a) + ", "s + dez->f);

	utils::globals::logger.log("Printing async\nprova\nprova");
	utils::globals::logger.flush();
	while (true)
		{
		using namespace std::literals::chrono_literals;
		std::this_thread::sleep_for(.1s);
		utils::globals::logger.log(std::to_string(asy->a) + ", "s + asy->f);
		}
	}

int main()
	{
	namespace ugv = utils::graphics::vulkan;
	
	try
		{
		ugv::core::manager manager;
	
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

		ugv::renderer::rectangle_renderpass tmp_renderpass{manager};

		ugv::core::renderer renderer {manager, window , tmp_renderpass};

		auto closer{ manager.get_closer() };
	
		iige::loop::variable_fps_and_game_speed loop;


		auto draw_callback = [&manager, &window, &renderer, &tmp_renderpass](float delta_time, float interpolation) -> void
			{
			if (window.width && window.height)
				{
				auto frame{renderer.begin_frame(delta_time)};

				frame.tmp_record_commands(delta_time);
				}
			};

		window.resize_redraw_callback = [&window, &loop]() { draw_callback(loop.update_delta_time(), loop.update_interpolation()); };

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