#pragma once

#include "../dependencies.h"

#include <vector>

namespace utils::graphics::vulkan::core
	{

	class swapchain_chosen_details
		{
		public:
			swapchain_chosen_details() = default;
			swapchain_chosen_details(vk::SurfaceCapabilitiesKHR capabilities, std::vector<vk::SurfaceFormatKHR> formats, std::vector<vk::PresentModeKHR> present_modes);

			const vk::SurfaceFormatKHR& get_format() const noexcept;
			const vk::PresentModeKHR& get_present_mode() const noexcept;
			uint32_t get_image_count() const noexcept;

		private:
			vk::SurfaceFormatKHR format;
			vk::PresentModeKHR   present_mode;
			uint32_t             image_count;

			static vk::SurfaceFormatKHR choose_format(const std::vector<vk::SurfaceFormatKHR>& available_formats) noexcept;

			static vk::PresentModeKHR choose_present_mode(const std::vector<vk::PresentModeKHR> available_present_modes) noexcept;

			static uint32_t choose_image_count(const vk::SurfaceCapabilitiesKHR& capabilities) noexcept;


		};
	}