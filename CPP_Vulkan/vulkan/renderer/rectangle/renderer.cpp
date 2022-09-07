#include "renderer.h"

#include <array>
#include <utils/compilation/debug.h>

namespace utils::graphics::vulkan::renderer
	{
	rectangle_renderer::rectangle_renderer(const core::manager& manager) :
		vertex_shader{ core::shader_vertex::from_glsl_file(manager.getter(this).device(), "data/shaders/rectangle/shader.vert") },
		fragment_shader{ core::shader_fragment::from_glsl_file(manager.getter(this).device(), "data/shaders/rectangle/shader.frag") },
		vk_renderpass{ create_renderpass(manager) },
		vk_pipeline{ create_pipeline(manager, vk_renderpass.get(), vertex_shader, fragment_shader) }
		{}

	void rectangle_renderer::resize(const core::manager& manager, const window::window& window)
		{
		vk_framebuffers = create_framebuffers(manager, window);
		}

	void rectangle_renderer::draw(core::manager& manager, const window::window& window, float delta_time)
		{
		if constexpr (utils::compilation::debug)
			{
			if (vk_framebuffers.empty()) { throw std::runtime_error{"You forgot to call resize on the current window."}; }
			}

		auto current_flying_frame { manager.getter(this).flying_frames_pool().get() };
		auto& device              { manager.getter(this).device() };
		auto& swapchain           { window.get_swapchain() };

		if (device.waitForFences(1, &current_flying_frame.vk_fence_frame_in_flight, VK_TRUE, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess)
			{
			throw std::runtime_error{ "Failed wait for fences" };
			}

		// TODO check if it doesnt explode your pc
		uint32_t image_index = swapchain.next_image(manager, current_flying_frame.vk_semaphore_image_available);
		current_flying_frame.vk_command_buffer.reset();

		record_commands(window, current_flying_frame.vk_command_buffer, image_index);

		vk::Semaphore          wait_semaphores  []{ current_flying_frame.vk_semaphore_image_available };
		vk::Semaphore          signal_semaphores[]{ current_flying_frame.vk_semaphore_render_finished };
		vk::PipelineStageFlags wait_stages      []{ vk::PipelineStageFlagBits::eColorAttachmentOutput };

		if(device.resetFences(1, &current_flying_frame.vk_fence_frame_in_flight) != vk::Result::eSuccess)
			{
			throw std::runtime_error{ "Failed reset fences" };
			}

		try
			{
			manager.getter(this).queues().get_graphics().queue.submit
			({ {
				.waitSemaphoreCount   { 1 },
				.pWaitSemaphores      { wait_semaphores },
				.pWaitDstStageMask    { wait_stages },
				.commandBufferCount   { 1 },
				.pCommandBuffers      { &current_flying_frame.vk_command_buffer },
				.signalSemaphoreCount { 1 },
				.pSignalSemaphores    { signal_semaphores },
			} }, current_flying_frame.vk_fence_frame_in_flight);
			}
		catch (vk::SystemError system_error) { throw std::runtime_error{ "Failed to submit to queue" }; }

		swapchain.present(manager, current_flying_frame.vk_semaphore_render_finished, image_index);
		}

	vk::UniqueRenderPass rectangle_renderer::create_renderpass(const core::manager& manager) const
		{
		vk::AttachmentDescription color_attachment_description; // attachment specified for color and/or depth buffers
		color_attachment_description.format = manager.getter(this).swapchain_chosen_details().get_format().format;
		color_attachment_description.samples = vk::SampleCountFlagBits::e1;
		color_attachment_description.loadOp = vk::AttachmentLoadOp::eClear;
		color_attachment_description.storeOp = vk::AttachmentStoreOp::eStore;
		color_attachment_description.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		color_attachment_description.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		color_attachment_description.initialLayout = vk::ImageLayout::eUndefined;
		color_attachment_description.finalLayout = vk::ImageLayout::ePresentSrcKHR;

		vk::AttachmentReference color_attachment_ref{};
		color_attachment_ref.attachment = 0; // Our array consists of a single VkAttachmentDescription, so its index is 0
		color_attachment_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::SubpassDescription subpass_description;
		subpass_description.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass_description.colorAttachmentCount = 1;
		subpass_description.pColorAttachments = &color_attachment_ref;

		vk::SubpassDependency dependency
			{
				.srcSubpass    { VK_SUBPASS_EXTERNAL },
				.dstSubpass    { 0 },
				.srcStageMask  { vk::PipelineStageFlagBits::eColorAttachmentOutput },
				.dstStageMask  { vk::PipelineStageFlagBits::eColorAttachmentOutput },
				.srcAccessMask { vk::AccessFlagBits::eNone },
				.dstAccessMask { vk::AccessFlagBits::eColorAttachmentWrite },
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
			ret = manager.getter(this).device().createRenderPassUnique(render_pass_create_info);
			}
		catch (vk::SystemError err)
			{
			throw std::runtime_error("Failed to create render pass!");
			}

		return ret;
		}

	vk::UniquePipeline rectangle_renderer::create_pipeline(const core::manager& manager, const vk::RenderPass& renderpass, const core::shader_vertex& vertex_shader, const core::shader_fragment& fragment_shader) const
		{
		std::vector<vk::DynamicState> dynamic_states;

		// TODO remember to call input assembly/vertex

		vk::PipelineViewportStateCreateInfo viewport_state;

		//Specific window size not needed since we're using a dynamic state that is set with the command buffer.
		dynamic_states.push_back(vk::DynamicState::eViewport);
		std::vector<vk::Viewport> viewports{vk::Viewport{.x{0}, .y{0}, .width{0.f}, .height{0.f}, .minDepth{0.f}, .maxDepth{1.f} }};
		viewport_state.viewportCount = viewports.size();
		viewport_state.pViewports = viewports.data();

		//Specific window size not needed since we're using a dynamic state that is set with the command buffer.
		dynamic_states.push_back(vk::DynamicState::eScissor);
		std::vector<vk::Rect2D> scissors{vk::Rect2D{.offset{0, 0}, .extent{0, 0}}};
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

		vk::PipelineDepthStencilStateCreateInfo depth_stencil{}; // TODO if we ever use it

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

		vk::PipelineLayoutCreateInfo pipeline_layout_info
			{
				.setLayoutCount { 0 }, // Optional
				.pushConstantRangeCount { 0 }, // Optional
			};

		vk::UniquePipelineLayout pipeline_layout;
		try
			{
			pipeline_layout = manager.getter(this).device().createPipelineLayoutUnique(pipeline_layout_info);
			}
		catch (vk::SystemError err)
			{
			throw std::runtime_error("Failed to create pipeline layout!");
			}

		// Render pass
		vk::PipelineVertexInputStateCreateInfo vertex_input_create_info
			{
				.vertexBindingDescriptionCount  { 0 },
				.pVertexBindingDescriptions     { nullptr }, // Optional TODO fill with actual bindings later on with tutorial
				.vertexAttributeDescriptionCount{ 0 },
				.pVertexAttributeDescriptions   { nullptr }, // Optional
			};
		vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info
			{
				.topology { vk::PrimitiveTopology::eTriangleList},
				.primitiveRestartEnable{ VK_FALSE },
			};

		std::vector<vk::PipelineShaderStageCreateInfo> shader_stages_infos{ vertex_shader.get_create_info(), fragment_shader.get_create_info() };

		vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info
			{
			//shader-stage setup
			.stageCount { 2 },
			.pStages { shader_stages_infos.data() },
			.pVertexInputState { &vertex_input_create_info },
			.pInputAssemblyState { &input_assembly_create_info },
			.pViewportState { &viewport_state },
			.pRasterizationState { &rasterizer },
			.pMultisampleState { &multisampling },
			.pDepthStencilState { nullptr }, // Optional
			.pColorBlendState { &color_blending },
			.pDynamicState { &dynamic_state_info },

			//fixed-function setup
			.layout { pipeline_layout.get() },

			//render-pass setup
			.renderPass { renderpass },
			.subpass { 0 }, //subpass index

			//pipeline-inheritance setup
			.basePipelineHandle { VK_NULL_HANDLE }, // Optional
			.basePipelineIndex { -1 }, // Optional
			};

		auto ret = manager.getter(this).device().createGraphicsPipelineUnique(nullptr, graphics_pipeline_create_info);
		if (ret.result != vk::Result::eSuccess)
			{
			throw std::runtime_error("Failed to create render pass!");
			}

		return std::move(ret.value);
		}

	vk::UniqueFramebuffer rectangle_renderer::create_framebuffer(const core::manager& manager, const window::window& window, size_t image_index) const
		{
		vk::UniqueFramebuffer ret;
		try
			{
			std::array<vk::ImageView, 1> attachments{ window.get_swapchain().get_image_view(image_index) };
			ret =
				{ manager.getter(this).device().createFramebufferUnique(
					vk::FramebufferCreateInfo
					{
						.renderPass      { vk_renderpass.get() },
						.attachmentCount { 1 },
						.pAttachments    { attachments.data() },
						.width           { window.width },
						.height          { window.height },
						.layers          { 1 },
					}
				) };
			}
		catch (vk::SystemError err)
			{
			throw std::runtime_error{ "Failed to create framebuffer!" };
			}
		return ret;
		}

	std::vector<vk::UniqueFramebuffer> rectangle_renderer::create_framebuffers(const core::manager& manager, const window::window& window) const
		{
		auto& swapchain{ window.get_swapchain() };
		std::vector<vk::UniqueFramebuffer> ret;
		ret.reserve(swapchain.size());

		for (size_t i = 0; i < swapchain.size(); i++)
			{
			vk::ImageView attachments[]{ swapchain.get_image_view(i) };
			auto framebuffer{ create_framebuffer(manager, window, i) };
			ret.push_back(std::move(framebuffer));
			}

		return ret;
		}

	void rectangle_renderer::record_commands(const window::window& window, vk::CommandBuffer& command_buffer, uint32_t image_index)
		{
		vk::CommandBufferBeginInfo beginInfo; // for now empty is okay (we dont need primary/secondary buffer info or flags)
		try {
			command_buffer.begin(beginInfo);
			}
		catch (vk::SystemError err) {
			throw std::runtime_error("failed to begin recording command buffer!");
			}

		//TODO get transparency color from window
		vk::ClearValue clearColor{ std::array<float, 4>{ 0.0f, 0.0f, 0.2f, 0.0f } };

		vk::RenderPassBeginInfo renderPassInfo
			{
				.renderPass  {vk_renderpass.get()},
				.framebuffer {vk_framebuffers[image_index].get()},
				.renderArea
				{
					.offset { 0, 0 },
					.extent { vk::Extent2D{window.width, window.height} },
				},
				.clearValueCount {1},
				.pClearValues {&clearColor},
			};

		command_buffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vk_pipeline.get());

		command_buffer.setViewport(0, vk::Viewport{ .x{0}, .y{0}, .width{static_cast<float>(window.width)}, .height{static_cast<float>(window.height)}, .minDepth{0.f}, .maxDepth{1.f} });
		command_buffer.setScissor(0, vk::Rect2D{ .offset{0, 0}, .extent{window.width, window.height} });

		command_buffer.draw(6, 1, 0, 0);

		command_buffer.endRenderPass();

		try {
			command_buffer.end();
			}
		catch (vk::SystemError err) {
			throw std::runtime_error("Failed to record command buffer!");
			}
		}
	}