#include "vulkan/core/utils.h"
#include "vulkan/core/manager.h"
#include "vulkan/core/shader.h"
#include "vulkan/window/window.h"
#include "vulkan/window/window_sized_image.h"
#include "vulkan/core/renderer.h"
#include "vulkan/renderer/rectangle/renderer.h"
#include "vulkan/renderer/3d/renderer.h"

#include "vulkan/core/model.h"

#include <utils_win32/transparent.h>
class vulkan_window : public utils::win32::window::t<utils::graphics::vulkan::window::window>, utils::win32::window::transparent<utils::win32::window::transparency_t::composition_attribute>
	{
	utils_devirtualize
	public:
		//using t<window_implementation_ts...>::t;
		vulkan_window(base::create_info c_info, utils::graphics::vulkan::core::manager& manager, std::vector<utils::observer_ptr< utils::graphics::vulkan::core::renderer>>& renderer_ptrs) :
			base{ c_info }, utils::graphics::vulkan::window::window{ manager, renderer_ptrs } {}
	};

int main()
	{
	namespace ugv = utils::graphics::vulkan;
	try
		{
		ugv::core::manager manager;

		//ugv::renderer::rectangle_renderer rect_renderer{manager};
		Model model = load_model("data/models/spyro/spyro.obj");
		ugv::renderer::renderer_3d renderer_3d{manager, model};

		std::vector<utils::observer_ptr<ugv::core::renderer>> renderer_ptrs{&renderer_3d};


		vulkan_window::initializer i;
		vulkan_window window
			{
			vulkan_window::create_info
				{
				.title{L"Sample Window Class"},
				},
			manager,
			renderer_ptrs
			};
		auto image = window.images.emplace(
			{
			.imageType = vk::ImageType::e2D,
			.format = vk::Format::eA8B8G8R8UnormPack32,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = vk::SampleCountFlagBits::e1,
			.tiling = vk::ImageTiling::eOptimal,
			.usage{vk::ImageUsageFlagBits::eInputAttachment},
			.initialLayout = vk::ImageLayout::eUndefined,
			});

		auto closer{manager.get_closer()};

		renderer_3d.resize(manager, window);
		while (window.is_open())
			{
			while (window.poll_event())
				{
				}
			if (window.is_open())
				{
				if(window.width & window.height)
					renderer_3d.draw(manager, window);
				//recz_renderer.draw(manager, window);
				}
			Sleep(100);
			}
		}
	catch (const std::exception& e)
		{
		ugv::core::logger.err(e.what());
		}
	return 0;
	}