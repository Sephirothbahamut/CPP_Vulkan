#pragma once

#include <utils/math/vec2.h>
#include <utils/math/vec3.h>
#include <utils/graphics/colour.h>

namespace utils::vulkan::resources
	{
	template <size_t size, typename floating_type>
		requires (size == 2 || size == 3) && std::is_floating_point_v<floating_type>
	struct vertex
		{
		utils::math::vec            <size, floating_type> position;
		utils::math::vec3           <floating_type>       normal;
		utils::math::vec2           <floating_type>       texture_coordinates;
		utils::graphics::colour::rgb<floating_type, 4>    colour;
		};

	template <typename floating_type>
	using vertex2 = vertex<2, floating_type>;
	template <typename floating_type>
	using vertex3 = vertex<3, floating_type>;

	template <size_t size>
	using vertexf = vertex<size, float >;
	template <size_t size>
	using vertexd = vertex<size, double>;

	using vertex2f = vertex<2, float >;
	using vertex2d = vertex<2, double>;

	using vertex3f = vertex<3, float >;
	using vertex3d = vertex<3, double>;
	}