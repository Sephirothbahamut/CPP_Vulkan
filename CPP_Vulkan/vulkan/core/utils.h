#pragma once

#include <optional>
#include <stdexcept>
#include <vulkan/vulkan.hpp>

namespace utils::graphics::vulkan::details
	{
	// TODO add concept to F for the signature: bool (const required_t&, const available_t&)
	template <typename required_t, typename available_t, typename F>
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
	}

namespace utils::graphics::vulkan
	{
	struct error : std::runtime_error { using std::runtime_error::runtime_error; };

	inline static utils::logger<utils::message> logger{"../logs/vulkan_log.txt"};
	}