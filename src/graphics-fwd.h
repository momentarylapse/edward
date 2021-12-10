/*
 * graphics.h
 *
 *  Created on: Nov 16, 2021
 *      Author: michi
 */

#pragma once


#if HAS_LIB_VULKAN
	#define USING_VULKAN
#else
	#define USING_OPENGL
#endif


#ifdef USING_VULKAN

// Vulkan

	namespace vulkan {
		class Texture;
		class Shader;
		class VertexBuffer;
		class FrameBuffer;
		class DepthBuffer;
		class Buffer;
		class UniformBuffer;
		enum class Alpha;
		enum class AlphaMode;
		class Pipeline;
		class DescriptorSet;
		class RenderPass;
		class CommandBuffer;
		class Semaphore;
		class Fence;
		class SwapChain;
		class DescriptorPool;
	}

	using Texture = vulkan::Texture;
	using Shader = vulkan::Shader;
	using VertexBuffer = vulkan::VertexBuffer;
	using FrameBuffer = vulkan::FrameBuffer;
	using DepthBuffer = vulkan::DepthBuffer;
	using CubeMap = vulkan::Texture; // TODO
	using Buffer = vulkan::Buffer;
	using UniformBuffer = vulkan::UniformBuffer;
	using Pipeline = vulkan::Pipeline;
	using CommandBuffer = vulkan::CommandBuffer;
	using DescriptorSet = vulkan::DescriptorSet;
	using RenderPass = vulkan::RenderPass;

	using Alpha = vulkan::Alpha;
	using AlphaMode = vulkan::AlphaMode;

#endif
#ifdef USING_OPENGL

// OpenGL

	namespace nix {
		class Texture;
		class Shader;
		class VertexBuffer;
		class FrameBuffer;
		class DepthBuffer;
		class CubeMap;
		class Buffer;
		class UniformBuffer;
		enum class Alpha;
		enum class AlphaMode;
	}

	using Texture = nix::Texture;
	using Shader = nix::Shader;
	using VertexBuffer = nix::VertexBuffer;
	using FrameBuffer = nix::FrameBuffer;
	using DepthBuffer = nix::DepthBuffer;
	using CubeMap = nix::CubeMap;
	using Buffer = nix::Buffer;
	using UniformBuffer = nix::UniformBuffer;

	using Alpha = nix::Alpha;
	using AlphaMode = nix::AlphaMode;

#endif
