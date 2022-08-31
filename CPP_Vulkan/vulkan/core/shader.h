#pragma once

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>

#include "../dependencies.h"

#include <shaderc/shaderc.hpp>
#include <spirv-tools/libspirv.hpp>
#include <spirv-headers/spirv.hpp>
#include <spirv_cross/spirv.hpp>

#include "manager.h"

namespace utils::graphics::vulkan::core
{
	namespace details
	{
		template <shaderc_shader_kind shader_kind>
		class shader
		{
		public:
			inline const vk::ShaderModule& get_shader_module() const noexcept
			{
				return vk_shader.get();
			}

			inline static shader from_spirv(const vk::Device& device, const std::vector<uint32_t>& spirv)
			{
				try
				{
					return
					{
						device.createShaderModuleUnique(
						{
							.flags    { vk::ShaderModuleCreateFlags()   },
							.codeSize { spirv.size() * sizeof(uint32_t) },
							.pCode    { spirv.data() },
						})
					};
				}
				catch (vk::SystemError err)
				{
					throw std::runtime_error("Failed to create shader module!");
				}
			}

			inline static shader from_glsl(const vk::Device& device, const std::string& glsl, const std::string& error_tag = "raw_data")
			{
				return from_spirv(device, compile_glsl(glsl, shader_kind, error_tag));
			}

			inline static shader from_spirv_file(const vk::Device& device, const std::filesystem::path& file)
			{
				return from_spirv(device, load_spirv(file));
			}

			inline static shader from_glsl_file(const vk::Device& device, const std::filesystem::path& file)
			{
				return from_glsl(device, load_glsl(file), file.filename().string());
			}

			inline vk::PipelineShaderStageCreateInfo get_create_info() const noexcept
			{
				return
				{
					.flags{vk::PipelineShaderStageCreateFlags()},
					.stage{to_shader_bits(shader_kind)},
					.module{vk_shader.get()},
					.pName{"main"},
				};
			}

		private:
			vk::UniqueShaderModule vk_shader;

			shader(vk::UniqueShaderModule&& vk_shader) : vk_shader{ std::move(vk_shader) }
			{}

			inline static const std::string load_glsl(const std::filesystem::path& source_path)
			{
				std::string         source_code;
				std::ifstream       source_file;
				std::stringstream source_stream;

				source_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

				try
				{
					source_file.open(source_path);
					source_stream << source_file.rdbuf();
					source_file.close();
					source_code = source_stream.str();
				}
				catch (const std::exception& e)
				{
					std::cerr << e.what() << '\n';
				}

				return source_code;
			}

			inline static const std::vector<uint32_t> load_spirv(const std::filesystem::path& spirv_path)
			{
				//open the file. With cursor at the end
				std::ifstream file(spirv_path, std::ios::ate | std::ios::binary);

				if (!file.is_open()) {
					throw std::runtime_error("failed to open file!");
				}

				//find what the size of the file is by looking up the location of the cursor
				//because the cursor is at the end, it gives the size directly in bytes
				size_t fileSize = (size_t)file.tellg();

				//spirv expects the buffer to be on uint32, so make sure to reserve an int vector big enough for the entire file
				std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

				//put file cursor at beginning
				file.seekg(0);

				//load the entire file into the buffer
				file.read((char*)buffer.data(), fileSize);

				//now that the file is loaded into the buffer, we can close it
				file.close();

				return buffer;
			}


			template <typename T>
			inline static bool successful_compilation(const shaderc::CompilationResult<T>& result) {
				return result.GetCompilationStatus() == shaderc_compilation_status_success;
			}

			template <typename T>
			inline static bool is_valid_spv(const shaderc::CompilationResult<T>& result) {
				if (!successful_compilation(result)) return false;
				size_t length_in_words = result.cend() - result.cbegin();
				if (length_in_words < 5) return false;
				const uint32_t* bytes = result.cbegin();
				return bytes[0] == spv::MagicNumber;
			}

			inline static std::vector<uint32_t> compile_glsl(const std::string& source_code, shaderc_shader_kind shader_kind, const std::string& error_tag)
			{
				shaderc::Compiler c{};
				shaderc::SpvCompilationResult compiled = c.CompileGlslToSpv(source_code, shader_kind, error_tag.c_str()); // N.B. the const char* file_name is just a tag, not a real path or file name
				//auto status = compiled.GetCompilationStatus(); TODO check if this info is needed to be passed to exception later
				if (!successful_compilation(compiled) || !is_valid_spv(compiled))
				{
					using namespace std::string_literals;
					throw std::runtime_error{ "glsl to SPIR-V compilation failed. Source file: \""s + error_tag + "\"." };
				}
				return std::vector<uint32_t>(compiled.begin(), compiled.end());
			}
		
			inline static consteval vk::ShaderStageFlagBits to_shader_bits(shaderc_shader_kind shader_kind)
			{
				switch (shader_kind)
				{
					case shaderc_shader_kind::shaderc_vertex_shader:
					case shaderc_shader_kind::shaderc_glsl_default_vertex_shader: 		   return vk::ShaderStageFlagBits::eVertex;
					case shaderc_shader_kind::shaderc_fragment_shader:
					case shaderc_shader_kind::shaderc_glsl_default_fragment_shader: 	   return vk::ShaderStageFlagBits::eFragment;
					case shaderc_shader_kind::shaderc_compute_shader:
					case shaderc_shader_kind::shaderc_glsl_default_compute_shader: 		   return vk::ShaderStageFlagBits::eCompute;
					case shaderc_shader_kind::shaderc_geometry_shader:
					case shaderc_shader_kind::shaderc_glsl_default_geometry_shader: 	   return vk::ShaderStageFlagBits::eGeometry;
					case shaderc_shader_kind::shaderc_tess_control_shader:
					case shaderc_shader_kind::shaderc_glsl_default_tess_control_shader:    return vk::ShaderStageFlagBits::eTessellationControl;
					case shaderc_shader_kind::shaderc_tess_evaluation_shader:
					case shaderc_shader_kind::shaderc_glsl_default_tess_evaluation_shader: return vk::ShaderStageFlagBits::eTessellationEvaluation;
					case shaderc_shader_kind::shaderc_raygen_shader:
					case shaderc_shader_kind::shaderc_glsl_default_raygen_shader: 		   return vk::ShaderStageFlagBits::eRaygenKHR;
					case shaderc_shader_kind::shaderc_anyhit_shader:
					case shaderc_shader_kind::shaderc_glsl_default_anyhit_shader: 		   return vk::ShaderStageFlagBits::eAnyHitKHR;
					case shaderc_shader_kind::shaderc_closesthit_shader:
					case shaderc_shader_kind::shaderc_glsl_default_closesthit_shader: 	   return vk::ShaderStageFlagBits::eClosestHitKHR;
					case shaderc_shader_kind::shaderc_miss_shader:
					case shaderc_shader_kind::shaderc_glsl_default_miss_shader: 		   return vk::ShaderStageFlagBits::eMissKHR;
					case shaderc_shader_kind::shaderc_intersection_shader:
					case shaderc_shader_kind::shaderc_glsl_default_intersection_shader:    return vk::ShaderStageFlagBits::eIntersectionKHR;
					case shaderc_shader_kind::shaderc_callable_shader:
					case shaderc_shader_kind::shaderc_glsl_default_callable_shader: 	   return vk::ShaderStageFlagBits::eCallableKHR;
					default:
						throw std::runtime_error{ "Im not runtime but you failed to choose the shader type :)" };
							//TODO check if u need other later
				}
			}
		};
	}

	using shader_fragment = details::shader<shaderc_shader_kind::shaderc_fragment_shader>;
	using shader_vertex   = details::shader<shaderc_shader_kind::shaderc_vertex_shader  >;
}