/*
 * base.h
 *
 *  Created on: 21 Nov 2021
 *      Author: michi
 */


#pragma once

#include "Renderer.h"
#include <lib/ygraphics/graphics-fwd.h>

struct GLFWwindow;

namespace xhui {
class Painter;
}

namespace yrenderer {

class ShaderManager;
class TextureManager;
class MaterialManager;
class Material;

struct Context {

	ygfx::Context* context;

	ShaderManager* shader_manager = nullptr;
	TextureManager* texture_manager = nullptr;
	MaterialManager* material_manager = nullptr;

	ygfx::Texture* tex_white;
	ygfx::Texture* tex_black;

	Array<int> gpu_timestamp_queries;

#ifdef USING_VULKAN
	vulkan::Instance* instance = nullptr;
	vulkan::DescriptorPool* pool;
	vulkan::Device* device;
#endif

	void _create_default_textures();

	void create_managers(const Path& texture_dir, const Path& shader_dir, const Path& material_dir);
	shared<ygfx::Texture> load_texture(const Path& path) const;
	xfer<Material> load_material(const Path &filename) const;

	shared<ygfx::Shader> load_shader(const Path& path) const;
	xfer<ygfx::Shader> create_shader(const string &source) const;
	shared<ygfx::Shader> load_surface_shader(const Path& path, const string &render_path, const string &vertex_module, const string &geometry_module) const;
	void load_shader_module(const Path& path) const;

	void reset_gpu_timestamp_queries();

	void gpu_timestamp(const RenderParams& params, int channel);
	void gpu_timestamp_begin(const RenderParams& params, int channel);
	void gpu_timestamp_end(const RenderParams& params, int channel);
	Array<float> gpu_read_timestamps();

	void gpu_flush();
};

Context* api_init_glfw(GLFWwindow* window);
#ifdef USING_VULKAN
Context* api_init_external(vulkan::Instance* instance, vulkan::Device* device);
#endif
Context* api_init_xhui(xhui::Painter* p);
void api_end(Context* ctx);

static constexpr int MAX_TIMESTAMP_QUERIES = 4096;


}
