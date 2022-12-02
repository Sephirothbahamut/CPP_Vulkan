#pragma once

#include <utils/math/vec2.h>
#include <utils/math/vec3.h>
#include <utils/graphics/colour.h>

#include <utils/containers/std_hash.h>

namespace utils::graphics::vulkan::resources
	{
	template <size_t size, std::floating_point floating_type>
		requires (size == 2 || size == 3)
	struct vertex
		{
		using position_t            = utils::math::vec            <floating_type, size>;
		using normal_t              = utils::math::vec3           <floating_type>;
		using texture_coordinates_t = utils::math::vec2           <floating_type>;
		using colour_t              = utils::graphics::colour::rgb<floating_type, 4>;

		position_t            position;
		normal_t              normal;
		texture_coordinates_t texture_coordinates;
		colour_t              colour;

		bool operator==(const vertex& other) const = default;
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

namespace std 
	{
	template <size_t size, typename floating_type>
	struct hash<utils::graphics::vulkan::resources::vertex<size, floating_type>>
		{
		size_t operator()(utils::graphics::vulkan::resources::vertex<size, floating_type> const& vertex) const
			{
			return
				hash<std::array<floating_type, size>>{}(static_cast<std::array<floating_type, size>>(vertex.position           )) ^
				hash<std::array<floating_type, 3   >>{}(static_cast<std::array<floating_type, 3   >>(vertex.normal             )) ^
				hash<std::array<floating_type, 2   >>{}(static_cast<std::array<floating_type, 2   >>(vertex.texture_coordinates)) ^
				hash<std::array<floating_type, 4   >>{}(static_cast<std::array<floating_type, 4   >>(vertex.colour             ));
			}
		};
	}