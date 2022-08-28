#include "vulkan/core/manager.h"
#include "vulkan/core/shader.h"
#include "vulkan/window/window.h"
#include "vulkan/window/window_sized_image.h"

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
	auto image = window.images.emplace
		({
		.imageType = vk::ImageType::e2D,
		.format = vk::Format::eA8B8G8R8UnormPack32,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = vk::SampleCountFlagBits::e1,
		.tiling = vk::ImageTiling::eOptimal,
		.usage{vk::ImageUsageFlagBits::eInputAttachment},
		.initialLayout = vk::ImageLayout::eUndefined,
		});

	while (window.is_open())
		{
		while (window.poll_event())
			{
			}
		}
	}