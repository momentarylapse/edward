//
// Created by michi on 23.11.24.
//

#ifndef Y_RENDERVIEWDATA_H
#define Y_RENDERVIEWDATA_H

#include <lib/base/base.h>
#include <lib/base/map.h>
#include <lib/base/pointer.h>
#include <lib/math/mat4.h>
#include <lib/image/color.h>
#include "../../../graphics-fwd.h"
#include "../../../world/Material.h"

struct SceneView;
class RenderParams;
class mat4;
class ShaderCache;
class Material;
enum class RenderPathType;

static constexpr int MAX_INSTANCES = 1<<11;

#ifdef USING_VULKAN

static constexpr int BINDING_TEX0 = 0;
static constexpr int BINDING_SHADOW0 = 5;
static constexpr int BINDING_SHADOW1 = 6;
static constexpr int BINDING_CUBE = 7;
static constexpr int BINDING_PARAMS = 8;
static constexpr int BINDING_LIGHT = 9;
static constexpr int BINDING_INSTANCE_MATRICES = 10;
static constexpr int BINDING_BONE_MATRICES = 11;

#endif

struct UBO {
	// matrix
	mat4 m,v,p;
	// material
	color albedo, emission;
	float roughness, metal;
	int dummy[2];
	int num_lights;
	int shadow_index;
	int num_surfels;
	int dummy2[2];
};

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
	void prepare_scene(SceneView* scene_view);
	void begin_draw();

	RenderPathType type;
	Material *material_shadow = nullptr; // ref to ShadowRenderer
	bool is_shadow_pass() const;

	UBO ubo;
#ifdef USING_VULKAN
	Array<RenderData> rda;
	int index = 0;
#endif

	void set_projection_matrix(const mat4& projection);
	void set_view_matrix(const mat4& view);
	void set_z(bool write, bool test);
	void set_wire(bool enabled);
	void set_cull(CullMode mode);

	owned<UniformBuffer> ubo_light;
	mat4 shadow_proj;
	void update_lights();

	//Array<UBOLight> lights;
	//mat4 shadow_proj;

	SceneView* scene_view = nullptr;
	RenderData rd;
	RenderData& start(const RenderParams& params, const mat4& matrix,
	                  Shader* shader, const Material& material, int pass_no,
	                  PrimitiveTopology top, VertexBuffer *vb);


	base::map<Material*, ShaderCache> multi_pass_shader_cache[4];
	// material as id!
	Shader* get_shader(Material* material, int pass_no, const string& vertex_shader_module, const string& geometry_shader_module);
};

#endif //Y_RENDERVIEWDATA_H
