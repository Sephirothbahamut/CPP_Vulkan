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
#include "iige/resources_manager_sync.h"
#include "iige/resources_manager_async.h"
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



int main()
	{
	using namespace std::string_literals;
	namespace ugv = utils::graphics::vulkan;
	//iige::resource::manager<thing_t, thing_t::create_info> rm_thing;
	iige::resource::manager_async<thong_t> rm_thong{ []() {return thong_t{9, "default"}; } };

	rm_thong.factories.emplace("pippo", []() {return thong_t{ 2, "pippo" }; });
	
	auto def{ rm_thong.load_async("sdbsdvdssdvd") };
	auto dez{ rm_thong.load_async("pippo") };
	auto dex{ rm_thong.load_async("pippo") };
	
	auto scd{ rm_thong.load_async("scd", []() {return thong_t{0, "scd"}; }) };
	auto qwe{ rm_thong.load_async("qwe", []() {return thong_t{3, "qwe"}; }) };
	
	auto asy{ rm_thong.load_async("asy", []() {return thong_t{1, "asy"}; }) };
	
	utils::globals::logger.log("Printing default");
	utils::globals::logger.log("def: " + std::to_string(def->a) + ", "s + def->f + "\n"s);
	utils::globals::logger.log("dez: " + std::to_string(dez->a) + ", "s + dez->f + "\n"s);
	utils::globals::logger.log("dex: " + std::to_string(dex->a) + ", "s + dex->f + "\n"s);
	utils::globals::logger.log("scd: " + std::to_string(scd->a) + ", "s + scd->f + "\n"s);
	utils::globals::logger.log("qwe: " + std::to_string(qwe->a) + ", "s + qwe->f + "\n"s);
	utils::globals::logger.log("asy: " + std::to_string(asy->a) + ", "s + asy->f + "\n"s);
	
	utils::globals::logger.log("Remapping def to dez");
	def.remap(dez);
	utils::globals::logger.log("def 2: " + std::to_string(def->a) + ", "s + def->f + "\n"s);
	
	utils::globals::logger.log("Unloading pippo");
	rm_thong.unload_sync("pippo");

	utils::globals::logger.log("dex u: " + std::to_string(dex->a) + ", "s + dex->f + "\n"s);
	
	utils::globals::logger.log("Reloading pippo");
	rm_thong.load_sync("pippo");
	
	utils::globals::logger.log("dex r: " + std::to_string(dex->a) + ", "s + dex->f + "\n"s);
	utils::globals::logger.log("dez r: " + std::to_string(dez->a) + ", "s + dez->f + "\n"s);

	utils::globals::logger.log("Printing async");
	while(true)
		{
		using namespace std::literals::chrono_literals;
		std::this_thread::sleep_for(.1s);
		utils::globals::logger.log(std::to_string(asy->a) + ", "s + asy->f + "\n"s);
		}
	
	//utils::globals::logger.flush(); TODO
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


		//iige::resource::manager<ugv::core::shader_fragment, std::function<ugv::core::shader_fragment()>> rm_sh{ [&]() { return ugv::core::shader_fragment::from_spirv_file(manager.device.get(), "shader.frag"); }};
		//rm_sh.load_async("pippo", [&]() { return ugv::core::shader_fragment::from_spirv_file(manager.device.get(), "shader.vert"); });

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