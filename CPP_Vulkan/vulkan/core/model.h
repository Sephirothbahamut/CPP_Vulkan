#pragma once
#include "../dependencies.h"

#include <utils/math/vec2.h>
#include <utils/math/vec3.h>
#include <utils/graphics/colour.h>

#include <tiny_obj_loader.h>

//add the include for glm to get matrices
#include <glm/glm.hpp>

struct Mesh_Push_Constants 
	{
	glm::vec4 data;
	glm::mat4 render_matrix;
	};

struct Vertex
	{
	utils::math::vec3f position;
	utils::graphics::colour::rgb<float, 4> color;
	utils::math::vec3f normal;
	utils::math::vec2<float> tex_coord;

	bool operator==(const Vertex& other) const
		{
		return position == other.position && color == other.color && tex_coord == other.tex_coord;
		}

	static vk::VertexInputBindingDescription getBindingDescription()
		{
		return
			{
				.binding   { 0 },
				.stride    { sizeof(Vertex) },
				.inputRate { vk::VertexInputRate::eVertex }
			};
		}

	static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions() {
		return
			{
				vk::VertexInputAttributeDescription
				{
					.location { 0 },
					.binding  { 0 },
					.format   { vk::Format::eR32G32B32Sfloat },
					.offset   { offsetof(Vertex, position) },
				},
				vk::VertexInputAttributeDescription
				{
					.location { 1 },
					.binding  { 0 },
					.format   { vk::Format::eR32G32B32A32Sfloat },
					.offset   { offsetof(Vertex, color) },
				},
				vk::VertexInputAttributeDescription
				{
					.location { 2 },
					.binding  { 0 },
					.format   { vk::Format::eR32G32B32Sfloat },
					.offset   { offsetof(Vertex, normal) },
				},
				vk::VertexInputAttributeDescription
				{
					.location { 3 },
					.binding  { 0 },
					.format   { vk::Format::eR32G32Sfloat },
					.offset   { offsetof(Vertex, tex_coord) },
				},
			};
		}
	};

namespace std
	{
	template<> struct hash<Vertex>
		{
		size_t operator()(Vertex const& vertex) const
			{
			return 1;
			}
		};
	}

struct Model
	{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	};

inline Model load_model(std::string model_path)
	{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, model_path.c_str())) {
		throw std::runtime_error(err);
		}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};

			vertex.position =
				{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
				};

			if (attrib.normals.size() >= (3 * index.normal_index + 2))
				{
				vertex.normal =
					{
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
					};
				}

			vertex.tex_coord =
				{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

			vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0)
				{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
				}

			indices.push_back(uniqueVertices[vertex]);
			}
		}

	return { std::move(vertices), std::move(indices) };
	}