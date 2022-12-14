#include "rectangle_renderpass.h"

#include <array>
#include <utils/compilation/debug.h>

#include "../../core/model.h"
#include "../../core/utils.h"

namespace utils::graphics::vulkan::renderer
	{
	rectangle_renderpass::rectangle_renderpass(core::manager& manager) :
		vk_unique_renderpass{create_renderpass(manager)},
		vertex_shader    { core::shader_vertex  ::from_file_spirv(manager.get_device(), "data/shaders/rectangle/vert.spv") },
		fragment_shader  { core::shader_fragment::from_file_spirv(manager.get_device(), "data/shaders/rectangle/frag.spv") },
		vk_unique_pipeline_layout{ create_pipeline_layout(manager) },
		vk_unique_pipeline{ create_pipeline(manager, vk_unique_renderpass.get(), vertex_shader, fragment_shader) },

		vk_unique_staging_vertex_buffer{ create_buffer(manager, vk::BufferUsageFlagBits::eTransferSrc, sizeof(vertices[0]) * vertices.size()) },
		vk_unique_staging_vertex_memory{ create_memory(manager, vk_unique_staging_vertex_buffer.get(), vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) },

		vk_unique_vertex_buffer{ create_buffer(manager, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, sizeof(vertices[0]) * vertices.size()) },
		vk_unique_vertex_memory{ create_memory(manager, vk_unique_vertex_buffer.get(), vk::MemoryPropertyFlagBits::eDeviceLocal) }

		{
		const auto& device{ manager.get_device() };
		device.bindBufferMemory(vk_unique_staging_vertex_buffer.get(), vk_unique_staging_vertex_memory.get(), 0);
		device.bindBufferMemory(vk_unique_vertex_buffer.get(), vk_unique_vertex_memory.get(), 0);

		fill_staging_memory(manager, vk_unique_staging_vertex_buffer.get(), vk_unique_staging_vertex_memory.get(), vertices);

		copy_buffer(manager, vk_unique_staging_vertex_buffer.get(), vk_unique_vertex_buffer.get(), sizeof(vertices[0])* vertices.size());
		}


	vk::UniqueRenderPass rectangle_renderpass::create_renderpass(const core::manager& manager) const
		{
		// color attachment 
		vk::AttachmentDescription color_attachment_description
			{
			.format { manager.get_swapchain_chosen_details().get_format().format },
			.samples { vk::SampleCountFlagBits::e1 },
			.loadOp { vk::AttachmentLoadOp::eClear },
			.storeOp { vk::AttachmentStoreOp::eStore },
			.stencilLoadOp { vk::AttachmentLoadOp::eDontCare },
			.stencilStoreOp { vk::AttachmentStoreOp::eDontCare },
			.initialLayout { vk::ImageLayout::eUndefined },
			.finalLayout { vk::ImageLayout::ePresentSrcKHR },
			};

		// color attachment is index 0
		vk::AttachmentReference color_attachment_ref
			{
			.attachment { 0 },
			.layout { vk::ImageLayout::eColorAttachmentOptimal },
			};

		vk::SubpassDescription subpass_description
			{
			.pipelineBindPoint { vk::PipelineBindPoint::eGraphics },
			.colorAttachmentCount { 1 },
			.pColorAttachments { &color_attachment_ref },
			};

		vk::SubpassDependency dependency
			{
				.srcSubpass    { VK_SUBPASS_EXTERNAL },
				.dstSubpass    { 0 },
				.srcStageMask  { vk::PipelineStageFlagBits::eColorAttachmentOutput},
				.dstStageMask  { vk::PipelineStageFlagBits::eColorAttachmentOutput},
				.srcAccessMask { vk::AccessFlagBits::eNone },
				.dstAccessMask { vk::AccessFlagBits::eColorAttachmentWrite},
			};

		vk::RenderPassCreateInfo render_pass_create_info
			{
				.attachmentCount{1},
				.pAttachments{&color_attachment_description},
				.subpassCount{1},
				.pSubpasses{&subpass_description},
				.dependencyCount{1},
				.pDependencies{&dependency},
			};

		vk::UniqueRenderPass ret;
		try
			{
			ret = manager.get_device().createRenderPassUnique(render_pass_create_info);
			}
		catch (vk::SystemError err)
			{
			throw std::runtime_error("Failed to create render pass!");
			}

		return ret;
		}

	vk::UniquePipelineLayout rectangle_renderpass::create_pipeline_layout(const core::manager& manager) const
		{

		vk::PipelineLayoutCreateInfo pipeline_layout_info
			{
				.setLayoutCount { 0 }, // Optional
				.pSetLayouts { nullptr }, // Optional
			};

		vk::UniquePipelineLayout pipeline_layout;
		try
			{
			pipeline_layout = manager.get_device().createPipelineLayoutUnique(pipeline_layout_info);
			}
		catch (vk::SystemError err)
			{
			throw std::runtime_error("Failed to create pipeline layout!");
			}
		return pipeline_layout;
		}

	vk::UniquePipeline rectangle_renderpass::create_pipeline(const core::manager& manager, const vk::RenderPass& renderpass, const core::shader_vertex& vertex_shader, const core::shader_fragment& fragment_shader) const
		{
		std::vector<vk::DynamicState> dynamic_states;

		// TODO remember to call input assembly/vertex

		vk::PipelineViewportStateCreateInfo viewport_state;

		//Specific window size not needed since we're using a dynamic state that is set with the command buffer.
		dynamic_states.push_back(vk::DynamicState::eViewport);
		std::vector<vk::Viewport> viewports{ vk::Viewport{.x{0}, .y{0}, .width{0.f}, .height{0.f}, .minDepth{0.f}, .maxDepth{1.f} } };
		viewport_state.viewportCount = viewports.size();
		viewport_state.pViewports = viewports.data();

		//Specific window size not needed since we're using a dynamic state that is set with the command buffer.
		dynamic_states.push_back(vk::DynamicState::eScissor);
		std::vector<vk::Rect2D> scissors{ vk::Rect2D{.offset{0, 0}, .extent{0, 0}} };
		viewport_state.scissorCount = scissors.size();
		viewport_state.pScissors = scissors.data();

		vk::PipelineDynamicStateCreateInfo dynamic_state_info;
		dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
		dynamic_state_info.pDynamicStates = dynamic_states.data();

		vk::PipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = vk::PolygonMode::eFill;
		rasterizer.lineWidth = 1.f;
		rasterizer.cullMode = vk::CullModeFlagBits::eBack;
		rasterizer.frontFace = vk::FrontFace::eClockwise;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		vk::PipelineMultisampleStateCreateInfo multisampling{};// if we ever use it, requires "?" GPU feature to work
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		vk::PipelineColorBlendAttachmentState color_blend_attachment{}; //N.B. this is per-framebuffer
		color_blend_attachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eOne; // Optional
		color_blend_attachment.dstColorBlendFactor = vk::BlendFactor::eZero; // Optional
		color_blend_attachment.colorBlendOp = vk::BlendOp::eAdd; // Optional
		color_blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne; // Optional
		color_blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero; // Optional
		color_blend_attachment.alphaBlendOp = vk::BlendOp::eAdd; // Optional

		vk::PipelineColorBlendStateCreateInfo color_blending{};
		color_blending.logicOpEnable = VK_FALSE;
		color_blending.logicOp = vk::LogicOp::eClear; // Optional
		color_blending.attachmentCount = 1;
		color_blending.pAttachments = &color_blend_attachment;
		color_blending.blendConstants[0] = 0.0f; // Optional
		color_blending.blendConstants[1] = 0.0f; // Optional
		color_blending.blendConstants[2] = 0.0f; // Optional
		color_blending.blendConstants[3] = 0.0f; // Optional

		// Render pass
		vk::VertexInputBindingDescription vertex_binding_description
			{
				.binding   { 0 },
				.stride    { sizeof(utils::math::vec2f) },
				.inputRate { vk::VertexInputRate::eVertex }
			};;
		vk::VertexInputAttributeDescription vertex_attribute_description
			{
				.location { 0 },
				.binding  { 0 },
				.format   { vk::Format::eR32G32Sfloat },
				.offset   { 0 },
			};
		vk::PipelineVertexInputStateCreateInfo vertex_input_create_info
			{
				.vertexBindingDescriptionCount  { 1 },
				.pVertexBindingDescriptions     { &vertex_binding_description }, // Optional TODO fill with actual bindings later on with tutorial
				.vertexAttributeDescriptionCount{ 1 },
				.pVertexAttributeDescriptions   { &vertex_attribute_description }, // Optional
			};
		vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info
			{
				.topology { vk::PrimitiveTopology::eTriangleList},
				.primitiveRestartEnable{ VK_FALSE },
			};

		std::vector<vk::PipelineShaderStageCreateInfo> shader_stages_infos{ vertex_shader.get_create_info(), fragment_shader.get_create_info() };

		vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info
			{
			//shader-stagesetup
			.stageCount{2},
			.pStages{shader_stages_infos.data()},
			.pVertexInputState{&vertex_input_create_info},
			.pInputAssemblyState{&input_assembly_create_info},
			.pViewportState{&viewport_state},
			.pRasterizationState{&rasterizer},
			.pMultisampleState{&multisampling},
			.pDepthStencilState{nullptr},//Optional
			.pColorBlendState{&color_blending},
			.pDynamicState{&dynamic_state_info},

			//fixed-functionsetup
			.layout { vk_unique_pipeline_layout.get() },

			//render-pass setup
			.renderPass { renderpass },
			.subpass { 0 }, //subpass index

			//pipeline-inheritance setup
			.basePipelineHandle { VK_NULL_HANDLE }, // Optional
			.basePipelineIndex { -1 }, // Optional
			};

		auto ret = manager.get_device().createGraphicsPipelineUnique(nullptr, graphics_pipeline_create_info);
		if (ret.result != vk::Result::eSuccess)
			{
			throw std::runtime_error("Failed to create render pass!");
			}

		return std::move(ret.value);
		}

		vk::UniqueBuffer rectangle_renderpass::create_buffer(const core::manager& manager, vk::BufferUsageFlags usage_flags, size_t size) const
			{
			vk::BufferCreateInfo bufferInfo
				{
				.size       {size},
				.usage      {usage_flags},
				.sharingMode{vk::SharingMode::eExclusive},
				};
			return manager.get_device().createBufferUnique(bufferInfo);
			}

		vk::UniqueDeviceMemory rectangle_renderpass::create_memory(const core::manager& manager, const vk::Buffer& buffer, vk::MemoryPropertyFlags mem_props_flags) const
			{
			const auto& device{ manager.get_device() };

			vk::MemoryRequirements mem_requirements{ device.getBufferMemoryRequirements(buffer) };

			vk::MemoryAllocateInfo alloc_info
				{
				.allocationSize{mem_requirements.size},
				.memoryTypeIndex{core::details::find_memory_type(manager.get_physical_device(), mem_requirements.memoryTypeBits, mem_props_flags)}
				};
			return device.allocateMemoryUnique(alloc_info);
			}

		void rectangle_renderpass::copy_buffer(core::manager& manager, const vk::Buffer& src, vk::Buffer dst, size_t size)
			{
			const auto& device{ manager.get_device() };
			const auto& graphics_queue{ manager.get_queues().get_graphics().queue };

			vk::CommandBufferAllocateInfo alloc_info
				{
				.commandPool{manager.get_memory_op_command_pool()},
				.level{vk::CommandBufferLevel::ePrimary },
				.commandBufferCount{1},
				};

			vk::UniqueCommandBuffer command_buffer{ std::move(device.allocateCommandBuffersUnique(alloc_info)[0]) };

			vk::CommandBufferBeginInfo beginInfo
				{
				.flags {vk::CommandBufferUsageFlagBits::eOneTimeSubmit}
				};

			command_buffer.get().begin(beginInfo);

			vk::BufferCopy copyRegion
				{
				.srcOffset{0   }, // Optional
				.dstOffset{0   }, // Optional
				.size     {size}
				};

			command_buffer.get().copyBuffer(src, dst, copyRegion); //seeplide dovesp elode. cit dige sappiamos 
			command_buffer.get().end();

			vk::SubmitInfo submitInfo
				{
				.commandBufferCount { 1 },
				.pCommandBuffers { &command_buffer.get() },
				};


			graphics_queue.submit(1, &submitInfo, VK_NULL_HANDLE);
			graphics_queue.waitIdle();
			}

		// Record
		void rectangle_renderpass::record_commands(const window::window& window, vk::CommandBuffer& command_buffer, vk::Framebuffer framebuffer, float delta_time)
			{
			vk::ClearValue clear_color{std::array<float, 4>{0.0f, 0.0f, 0.2f, 0.0f}};

			utils::math::vec2u32 client_size{window.client_rect.size};
			vk::RenderPassBeginInfo renderpass_info
				{
				.renderPass  {vk_unique_renderpass.get()},
				.framebuffer {framebuffer},
				.renderArea
					{
					.offset {0, 0},
					.extent {vk::Extent2D{client_size.x, client_size.y}},
					},
				.clearValueCount {1},
				.pClearValues {&clear_color},
				};

			command_buffer.beginRenderPass(renderpass_info, vk::SubpassContents::eInline);


			command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vk_unique_pipeline.get());

			command_buffer.setViewport(0, vk::Viewport{.x{0}, .y{0}, .width{static_cast<float>(client_size.x)}, .height{static_cast<float>(client_size.y)}, .minDepth{0.f}, .maxDepth{1.f}});
			command_buffer.setScissor(0, vk::Rect2D{.offset{0, 0}, .extent{client_size.x, client_size.y}});

			vk::Buffer vertex_buffers[] = {vk_unique_vertex_buffer.get()};
			vk::DeviceSize offsets[] = {0};
			command_buffer.bindVertexBuffers(0, 1, vertex_buffers, offsets);

			command_buffer.draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0);

			command_buffer.endRenderPass();

			try { command_buffer.end(); }
			catch (vk::SystemError err) 
				{
				throw std::runtime_error("Failed to record command buffer!");
				}
			}
	}