//#include <chrono>
//
//#include <utils/logger.h>
//#include <utils/thread_pool.h>
//
//#include "iige/assets_manager.h"
//#include "iige/one_time_tasks.h"
//#include "vulkan/core/model.h"
//
//#include "iige/loop.h"
//
//struct thing_t
//	{
//	struct create_info { int a{5}; };
//	thing_t(const create_info& cinfo) : a{cinfo.a}
//		{
//		}
//	int a;
//	};
//struct thong_t
//	{
//	thong_t(int a, const std::string& f) : a{a}, f{f}
//		{
//		if (a == 0) { throw std::runtime_error{"hello"}; }
//		if (a == 1) {} std::this_thread::sleep_for(std::chrono::seconds(1));
//		}
//	int a;
//	std::string f;
//	};
//
//void test_asset_managers()
//	{
//	using namespace std::string_literals;
//
//	utils::thread_pool tp;
//	iige::one_time_tasks ott;
//
//	//iige::asset::manager_async<int> rm_int{ []() {return int{0}; } };
//	iige::asset::manager<thong_t> rm_thong{tp, ott, []() {return thong_t{9, "default"}; }};
//	iige::asset::manager<thing_t> rm_thing{tp, ott, []() {return thing_t{{}}; }};
//	iige::assets_manager rm{rm_thong, rm_thing};
//
//
//
//	auto def{rm_thong.load_async("sdbsdvdssdvd")};
//	auto dez{rm_thong.load_async("pippo", []() {return thong_t{13, "pippo"}; })};
//	auto dex{rm_thong.load_async("pippo")};
//
//	//auto scd{ rm_thong.get_containing_type().load_sync("wsdhajhsd") };
//	auto scd{rm_thong.load_sync("scd", []() {return thong_t{0, "scd"}; })};
//	auto qwe{rm_thong.load_async("qwe", []() {return thong_t{3, "qwe"}; })};
//
//	auto asy{rm_thong.load_async("asy", []() {return thong_t{1, "asy"}; })};
//
//	utils::globals::logger.log("Printing default");
//	utils::globals::logger.log("def: " + std::to_string(def->a) + ", "s + def->f);
//	utils::globals::logger.log("dez: " + std::to_string(dez->a) + ", "s + dez->f);
//	utils::globals::logger.log("dex: " + std::to_string(dex->a) + ", "s + dex->f);
//	utils::globals::logger.log("scd: " + std::to_string(scd->a) + ", "s + scd->f);
//	utils::globals::logger.log("qwe: " + std::to_string(qwe->a) + ", "s + qwe->f);
//	utils::globals::logger.log("asy: " + std::to_string(asy->a) + ", "s + asy->f);
//
//
//	utils::globals::logger.log("Unloading pippo");
//	rm_thong.unload_sync("pippo");
//
//	//utils::globals::logger.log("dex u: " + std::to_string(dex->a) + ", "s + dex->f);
//
//	utils::globals::logger.log("Reloading pippo");
//	rm_thong.load_sync("pippo");
//
//	utils::globals::logger.log("dex r: " + std::to_string(dex->a) + ", "s + dex->f);
//	utils::globals::logger.log("dez r: " + std::to_string(dez->a) + ", "s + dez->f);
//
//	utils::globals::logger.log("Printing async\nprova\nprova");
//	utils::globals::logger.flush();
//	while (true)
//		{
//		using namespace std::literals::chrono_literals;
//		std::this_thread::sleep_for(.1s);
//		utils::globals::logger.log(std::to_string(asy->a) + ", "s + asy->f);
//		}
//	}