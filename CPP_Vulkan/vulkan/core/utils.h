#pragma once

#include <utils/logger.h>
#include <utils/compilation/debug.h>

#include <vulkan/vulkan.hpp>

#include <optional>
#include <stdexcept>

namespace utils::graphics::vulkan::core::details
	{
	template <typename F, typename required_t, typename available_t>
	concept comparator = requires (F f, const required_t& rt, const available_t& at) { f(rt, at); };

	template <typename required_t, typename available_t, comparator<required_t, available_t> F>
	inline static std::vector<required_t> check_required_in_available(const std::vector<required_t>& required_vec, const std::vector<available_t>& available_vec, F lambda)
		{
		std::vector<required_t> ret;
		for (const auto& required : required_vec)
			{
			bool found = false;
			for (const auto& available : available_vec)
				{
				if (lambda(required, available)) { found = true; break; }
				}
			if (!found) { ret.push_back(required); }
			}
		return ret;
		}

	template <typename required_t, typename available_t, typename F>
	inline static std::optional<required_t> check_required_in_available_fast(const std::vector<required_t>& required_vec, const std::vector<available_t>& available_vec, F lambda)
		{
		for (const auto& required : required_vec)
			{
			bool found = false;
			for (const auto& available : available_vec)
				{
				if (lambda(required, available)) { found = true; break; }
				}
			if (!found) { return required; }
			}
		return std::nullopt;
		}

	struct validation_layers
		{
		inline static constexpr bool enabled{ utils::compilation::debug };
		inline static const std::vector<const char*> list
			{
				"VK_LAYER_KHRONOS_validation"
			};

		[[nodiscard]] static std::vector<const char*> check_missing()
			{
			if (!enabled) { return {}; }
			static auto available{ vk::enumerateInstanceLayerProperties() };
			return check_required_in_available(list, available, [](const auto& required, const auto& available) { return strcmp(required, available.layerName) == 0; });
			}
		};
	}

namespace utils::graphics::vulkan::core
	{
	struct error : std::runtime_error { using std::runtime_error::runtime_error; };

	inline static utils::logger<utils::message> logger{"../logs/vulkan_log.txt"};
	}