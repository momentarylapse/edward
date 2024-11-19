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
		class Instance;
		class Texture;
		class VolumeTexture;
		class StorageTexture;
		class CubeMap;
		class Shader;
		class VertexBuffer;
		class FrameBuffer;
		class DepthBuffer;
		class Buffer;
		class UniformBuffer;
		class StorageBuffer;
		enum class Alpha;
		enum class AlphaMode;
		enum class CullMode;
		class GraphicsPipeline;
		class ComputePipeline;
		class RayPipeline;
		class DescriptorSet;
		class RenderPass;
		class CommandBuffer;
		class Semaphore;
		class Fence;
		class SwapChain;
		class DescriptorPool;
		class Device;
		class AccelerationStructure;
	}

	class Context{};
	using Texture = vulkan::Texture;
	using Shader = vulkan::Shader;
	using VertexBuffer = vulkan::VertexBuffer;
	using FrameBuffer = vulkan::FrameBuffer;
	using DepthBuffer = vulkan::DepthBuffer;
	using CubeMap = vulkan::CubeMap;
	using VolumeTexture = vulkan::VolumeTexture;
	using ImageTexture = vulkan::StorageTexture;
	using Buffer = vulkan::Buffer;
	using UniformBuffer = vulkan::UniformBuffer;
	using ShaderStorageBuffer = vulkan::StorageBuffer;
	using GraphicsPipeline = vulkan::GraphicsPipeline;
	using ComputePipeline = vulkan::ComputePipeline;
	using RayPipeline = vulkan::RayPipeline;
	using CommandBuffer = vulkan::CommandBuffer;
	using DescriptorSet = vulkan::DescriptorSet;
	using RenderPass = vulkan::RenderPass;

	using Alpha = vulkan::Alpha;
	using AlphaMode = vulkan::AlphaMode;
	using CullMode = vulkan::CullMode;

	enum class FogMode {
		LINEAR,
		EXP,
		EXP2
	};

#endif
#ifdef USING_OPENGL

// OpenGL

	namespace nix {
		class Context;
		class Texture;
		class Shader;
		class VertexBuffer;
		class FrameBuffer;
		class DepthBuffer;
		class CubeMap;
		class VolumeTexture;
		class ImageTexture;
		class Buffer;
		class UniformBuffer;
		class ShaderStorageBuffer;
		enum class Alpha;
		enum class AlphaMode;
		enum class CullMode;
		enum class FogMode;
	}

	using Context = nix::Context;
	using Texture = nix::Texture;
	using Shader = nix::Shader;
	using VertexBuffer = nix::VertexBuffer;
	using FrameBuffer = nix::FrameBuffer;
	using DepthBuffer = nix::DepthBuffer;
	using CubeMap = nix::CubeMap;
	using VolumeTexture = nix::VolumeTexture;
	using ImageTexture = nix::ImageTexture;
	using Buffer = nix::Buffer;
	using UniformBuffer = nix::UniformBuffer;
	using ShaderStorageBuffer = nix::ShaderStorageBuffer;

	using Alpha = nix::Alpha;
	using AlphaMode = nix::AlphaMode;
	using CullMode = nix::CullMode;
	using FogMode = nix::FogMode;
	class RenderPass{};

#endif

enum class PrimitiveTopology {
	TRIANGLES,
	LINES,
	LINESTRIP,
	POINTS
};
