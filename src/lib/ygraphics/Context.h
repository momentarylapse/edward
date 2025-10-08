#pragma once

#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include "graphics-fwd.h"
#include "font.h"

enum class ColorSpace;
struct color;

namespace ygfx {

class Context;

struct TextCache {
	string text;
	font::Face* face;
	float font_size;
	int age;
	Texture* texture;
#if HAS_LIB_VULKAN
	vulkan::DescriptorSet* dset;
#endif
	font::TextDimensions dimensions;
};

struct DrawingHelperData {
	explicit DrawingHelperData(Context*);
	void create_basic();
#ifdef USING_VULKAN
	void rebuild(RenderPass* render_pass);
#endif
	Context* context;
	VertexBuffer* vb = nullptr;
	Shader* shader = nullptr;
	Shader* shader_lines = nullptr;
#ifdef USING_VULKAN
	vulkan::CommandBuffer* cb = nullptr;
	vulkan::GraphicsPipeline* pipeline = nullptr;
	vulkan::GraphicsPipeline* pipeline_alpha = nullptr;
	vulkan::GraphicsPipeline* pipeline_lines = nullptr;
	vulkan::DescriptorPool* pool = nullptr;

	vulkan::DescriptorSet* dset = nullptr;
	vulkan::DescriptorSet* dset_lines = nullptr;


	Array<vulkan::VertexBuffer*> line_vbs;
	int num_line_vbs_used = 0;
	Array<vulkan::VertexBuffer*> line_vbs_with_color;
	int num_line_vbs_with_color_used = 0;
	vulkan::VertexBuffer* get_line_vb(bool with_color = false);

	Array<vulkan::DescriptorSet*> descriptor_sets;
	int descriptor_sets_used = 0;
	vulkan::DescriptorSet* get_descriptor_set(Texture* texture);
#else
	Texture* tex_text = nullptr;
	Texture* tex_xxx = nullptr;
	Shader* shader_round = nullptr;
#endif
	void reset_frame();

	Array<TextCache> text_caches;
	TextCache& get_text_cache(const string& text, font::Face* face, float font_size, float ui_scale);
	void iterate_text_caches();
};

class Context {
public:
#if HAS_LIB_VULKAN
	explicit Context(vulkan::Instance* instance, vulkan::Device* device);
#else
	explicit Context(nix::Context* ctx);
#endif
	~Context();

#if HAS_LIB_VULKAN
	vulkan::Instance* instance = nullptr;
	vulkan::Device* device = nullptr;
#else
	nix::Context* ctx = nullptr;
#endif

	void make_current();

	ColorSpace color_space_shaders;
	ColorSpace color_space_input; // how to interpret parameters?

	color color_input_to_shaders(const color& c) const;

	DrawingHelperData* _create_auxiliary_stuff();

	Texture* tex_white = nullptr;
	Texture* tex_black = nullptr;
	void _create_default_textures();
};

font::TextDimensions& get_cached_text_dimensions(const string& text, font::Face* face, float font_size, float ui_scale);

}

