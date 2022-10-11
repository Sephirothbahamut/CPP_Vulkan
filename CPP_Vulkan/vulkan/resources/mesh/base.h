#pragma once

#include <fstream>
#include <filesystem>
#include <vector>
#include <unordered_map>

#include <tiny_obj_loader.h> //TODO replace

//TODO look into glTF 2.0 https://github.com/KhronosGroup/glTF
//Check if it can support 2d vertices, it should support colors

#include <utils/logger.h>
#include "../vertex.h"

namespace utils::graphics::vulkan::resources::mesh
	{
	template <size_t size, typename floating_type>
		requires (size == 2 || size == 3) && std::is_floating_point_v<floating_type>
	class base
		{
		using vertex_t = vertex<size, floating_type>;
		public:
			base           (base&& move) noexcept = default;
			base& operator=(base&& move) noexcept = default;

			inline static base from_file_obj(const std::filesystem::path& path)
				{
				std::vector<vertex_t> vertices;
				std::vector<uint32_t> indices;

				tinyobj::attrib_t                attrib;
				std::vector<tinyobj::shape_t>    shapes;
				std::vector<tinyobj::material_t> materials;

				std::string err;

				if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.string().c_str()))
					{
					throw std::runtime_error(err);
					}

				std::unordered_map<vertex_t, uint32_t> unique_vertices{};

				for (const auto& shape : shapes)
					{
					for (const auto& index : shape.mesh.indices)
						{
						vertex_t vertex{};

						if constexpr (size == 2)
							{
							vertex.position =
								{
								attrib.vertices[3 * index.vertex_index + 0],
								attrib.vertices[3 * index.vertex_index + 1]
								};
							}
						else if constexpr (size == 3)
							{
							vertex.position =
								{
								attrib.vertices[3 * index.vertex_index + 0],
								attrib.vertices[3 * index.vertex_index + 1],
								attrib.vertices[3 * index.vertex_index + 2]
								};
							}

						if (attrib.normals.size() >= (3 * index.normal_index + 2))
							{
							vertex.normal =
								{
								attrib.normals[3 * index.normal_index + 0],
								attrib.normals[3 * index.normal_index + 1],
								attrib.normals[3 * index.normal_index + 2]
								};
							}
						if (attrib.texcoords.size() >= (2 * index.texcoord_index + 1)) //TODO test? we handwrote 2* and +1 completely out of the blue hoping it would work lol
							{
							vertex.texture_coordinates =
								{
								attrib.texcoords[2 * index.texcoord_index + 0],
								1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
								};
							}

						vertex.colour = {1.0f, 1.0f, 1.0f, 1.0f};

						if (unique_vertices.count(vertex) == 0)
							{
							unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
							vertices.push_back(vertex);
							}

						indices.push_back(unique_vertices[vertex]);
						}
					}

				return {std::move(vertices), std::move(indices)};
				}


			std::vector<vertex_t> vertices;
			std::vector<uint32_t> indices;

			base clone() { return {*this}; }

		private:
			base(std::vector<vertex_t>&& vertices, std::vector<uint32_t>&& indices) : vertices{std::move(vertices)}, indices{std::move(indices)} {}
			base(const base& copy) = default;
			base& operator=(const base& copy) = default;
		};
	
	}