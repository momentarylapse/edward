//
// Created by michi on 23.11.24.
//

#ifndef Y_RENDERVIEWDATA_H
#define Y_RENDERVIEWDATA_H

#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <lib/math/mat4.h>
#include <lib/image/color.h>
#include "../../../graphics-fwd.h"

struct SceneView;
class RenderParams;
class mat4;
class ShaderCache;
class Material;
enum class RenderPathType;


#ifdef USING_VULKAN

static constexpr int BINDING_TEX0 = 0;
static constexpr int BINDING_SHADOW0 = 5;
static constexpr int BINDING_SHADOW1 = 6;
static constexpr int BINDING_CUBE = 7;
static constexpr int BINDING_PARAMS = 8;
static constexpr int BINDING_LIGHT = 9;
static constexpr int BINDING_INSTANCE_MATRICES = 10;
static constexpr int BINDING_BONE_MATRICES = 11;

static constexpr int MAX_INSTANCES = 1<<11;

struct UBO {
	// matrix
	mat4 m,v,p;
	// material
	color albedo, emission;
	float roughness, metal;
	int dummy[2];
	int num_lights;
	int shadow_index;
	int dummy2[2];
};
#endif

struct RenderData {
#ifdef USING_VULKAN
	UniformBuffer* ubo;
	DescriptorSet* dset;
#endif
	void set_textures(const SceneView& scene_view, const Array<Texture*>& tex);
	void apply(const RenderParams& params);
};

struct RenderViewData {
	RenderViewData();
	void begin_scene(SceneView* scene_view);

#ifdef USING_OPENGL
	struct UBO {
		int num_lights, shadow_index;
	} ubo;
#endif
#ifdef USING_VULKAN
	Array<RenderData> rda;
	int index = 0;
	UBO ubo;
#endif

	void set_projection_matrix(const mat4& projection);
	void set_view_matrix(const mat4& view);

	owned<UniformBuffer> ubo_light;

	SceneView* scene_view = nullptr;
	RenderData rd;
	RenderData& start(const RenderParams& params, RenderPathType type, const mat4& matrix,
	                  ShaderCache& shader_cache, const Material& material, int pass_no,
	                  const string& vertex_shader_module, const string& geometry_shader_module,
	                  PrimitiveTopology top, VertexBuffer *vb);
};

#endif //Y_RENDERVIEWDATA_H
