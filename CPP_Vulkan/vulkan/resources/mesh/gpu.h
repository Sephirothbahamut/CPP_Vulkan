#pragma once

#include <vulkan/vulkan.hpp>
#include "../../core/manager.h"
#include "base.h"

namespace utils::graphics::vulkan::resources::mesh
	{
	template <size_t size, typename floating_type>
		requires (size == 2 || size == 3) && std::is_floating_point_v<floating_type>
	class gpu
		{
		using vertex_t = vertex<size, floating_type>;
		public:
			gpu(vk::CommandBuffer& memory_operations_buffer, const base<size, floating_type>& base, vulkan::core::manager& manager) :
				vk_unique_staging_vertex_buffer{create_cpu_buffer(manager, vk::BufferUsageFlagBits::eTransferSrc, sizeof(base.vertices[0]) * base.vertices.size())},
				vk_unique_staging_vertex_memory{create_gpu_memory(manager, vk_unique_staging_vertex_buffer.get(), vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) },

				vk_unique_staging_index_buffer {create_cpu_buffer(manager, vk::BufferUsageFlagBits::eTransferSrc, sizeof(base.indices[0]) * base.indices.size()) },
				vk_unique_staging_index_memory {create_gpu_memory(manager, vk_unique_staging_index_buffer.get(), vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) },
		
				vk_unique_vertex_buffer        {create_cpu_buffer(manager, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, sizeof(base.vertices[0]) * base.vertices.size()) },
				vk_unique_vertex_memory        {create_gpu_memory(manager, vk_unique_vertex_buffer.get(), vk::MemoryPropertyFlagBits::eDeviceLocal) },

				vk_unique_index_buffer         {create_cpu_buffer(manager, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, sizeof(base.indices[0]) * base.indices.size()) },
				vk_unique_index_memory         {create_gpu_memory(manager, vk_unique_index_buffer.get(), vk::MemoryPropertyFlagBits::eDeviceLocal) }
		
				{
				const auto& device{ manager.get_device() };
				device.bindBufferMemory(vk_unique_staging_vertex_buffer.get(), vk_unique_staging_vertex_memory.get(), 0);
				device.bindBufferMemory(vk_unique_vertex_buffer        .get(), vk_unique_vertex_memory        .get(), 0);

				device.bindBufferMemory(vk_unique_staging_index_buffer .get(), vk_unique_staging_index_memory .get(), 0);
				device.bindBufferMemory(vk_unique_index_buffer         .get(), vk_unique_index_memory         .get(), 0);
		
				fill_staging_memory(manager, vk_unique_staging_vertex_memory.get(), base.vertices);
				fill_staging_memory(manager, vk_unique_staging_index_memory .get(), base.indices );
		
				copy_buffer(memory_operations_buffer, vk_unique_staging_vertex_buffer.get(), vk_unique_vertex_buffer.get(), base.vertices.size() * sizeof(base.vertices[0]));
				copy_buffer(memory_operations_buffer, vk_unique_staging_index_buffer .get(), vk_unique_index_buffer .get(), base.indices .size() * sizeof(base.indices [0]));
				}

		private:
			vk::UniqueBuffer       vk_unique_staging_vertex_buffer;
			vk::UniqueDeviceMemory vk_unique_staging_vertex_memory;
			vk::UniqueBuffer       vk_unique_vertex_buffer;
			vk::UniqueDeviceMemory vk_unique_vertex_memory;
			vk::UniqueBuffer       vk_unique_staging_index_buffer;
			vk::UniqueDeviceMemory vk_unique_staging_index_memory;
			vk::UniqueBuffer       vk_unique_index_buffer;
			vk::UniqueDeviceMemory vk_unique_index_memory;

			vk::UniqueBuffer create_cpu_buffer(core::manager& manager, vk::BufferUsageFlags usage_flags, size_t size) const
				{
				vk::BufferCreateInfo buffer_info
					{
						.size        { size },
						.usage       { usage_flags },
						.sharingMode { vk::SharingMode::eExclusive },
					};
				return manager.get_device().createBufferUnique(buffer_info);
				}

			vk::UniqueDeviceMemory create_gpu_memory(core::manager& manager, const vk::Buffer& buffer, vk::MemoryPropertyFlags mem_props_flags) const
				{
				const auto& device{manager.get_device()};

				vk::MemoryRequirements mem_requirements{device.getBufferMemoryRequirements(buffer)};

				vk::MemoryAllocateInfo alloc_info
					{
					.allocationSize{mem_requirements.size},
					.memoryTypeIndex{core::details::find_memory_type(manager.get_physical_device(), mem_requirements.memoryTypeBits, mem_props_flags)}
					};
				return device.allocateMemoryUnique(alloc_info);
				}

			template <typename T>
			void fill_staging_memory(core::manager& manager, vk::DeviceMemory& memory, std::vector<T> in_data)
				{
				const auto& device{manager.get_device()};
				size_t buffer_size{sizeof(in_data[0]) * in_data.size()};

				utils::observer_ptr<void> data{device.mapMemory(memory, 0, buffer_size, vk::MemoryMapFlags{})};

				//std::copy(in_data.begin(), in_data.end(), data);
				memcpy(data, in_data.data(), buffer_size);

				device.unmapMemory(memory);
				}

			void copy_buffer(vk::CommandBuffer& memory_operations, vk::Buffer src, vk::Buffer dst, size_t size)
				{
				vk::BufferCopy copy_region
					{
					.srcOffset{ 0    }, // Optional
					.dstOffset{ 0    }, // Optional
					.size     { size }
					};
				
				memory_operations.copyBuffer(src, dst, copy_region); //seeplide dovesp elode. cit dige sappiamos //se esplide sappiamo dove esplode cit. dige //archeologia restaurativa :)
				}
		};
	}