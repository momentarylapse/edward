//
// Created by michi on 23.11.24.
//

#ifndef Y_RENDERVIEWDATA_H
#define Y_RENDERVIEWDATA_H

#include <lib/base/base.h>
#include <lib/base/map.h>
#include <lib/base/pointer.h>
#include <lib/math/mat4.h>
#include <lib/math/quaternion.h>
#include <lib/math/vec3.h>
#include <lib/image/color.h>
#include <graphics-fwd.h>
#include <world/Material.h>

class Camera;
struct SceneView;
class RenderParams;
class mat4;
class ShaderCache;
class Material;
enum class RenderPathType;

static constexpr int MAX_INSTANCES = 1<<11;


static constexpr int BINDING_TEX0 = 0;
static constexpr int BINDING_SHADOW0 = 5;
static constexpr int BINDING_SHADOW1 = 6;
static constexpr int BINDING_CUBE = 7;
static constexpr int BINDING_PARAMS = 8;
static constexpr int BINDING_LIGHT = 9;
static constexpr int BINDING_INSTANCE_MATRICES = 10;
static constexpr int BINDING_BONE_MATRICES = 11;


// per mesh
struct UBO {
	// matrix
	mat4 m,v,p;
	// material
	color albedo, emission;
	float roughness, metal;
	int dummy[2];
};

struct LightMetaData {
	int num_lights;
	int num_surfels;
	int dummy[2];
	mat4 shadow_proj[2];
};

// single "draw call"
struct RenderData {
#ifdef USING_VULKAN
	UniformBuffer* ubo;
	DescriptorSet* dset;
#else
	void set_material_x(const SceneView& scene_view, const Material& m, Shader* s, int pass_no);
#endif
	void set_texture(int binding, Texture* tex);
	void set_textures(const SceneView& scene_view, const Array<Texture*>& tex);
	void draw_triangles(const RenderParams& params, VertexBuffer* vb);
	void draw_instanced(const RenderParams& params, VertexBuffer* vb, int count);
	void draw(const RenderParams& params, VertexBuffer* vb, PrimitiveTopology topology);
};

// "draw call" manager (single scene/pass)
struct RenderViewData {
	RenderViewData();
	void begin_draw();

	SceneView* scene_view = nullptr;
	void set_scene_view(SceneView* scene_view);
	RenderPathType type;
	Material* material_shadow = nullptr; // ref to ShadowRenderer
	bool is_shadow_pass() const;

	vec3 view_pos;
	quaternion view_ang;
	UBO ubo;
#ifdef USING_VULKAN
	Array<RenderData> rda;
	int index = 0;
#else
	RenderData rd;
#endif

	void set_view(const RenderParams& params, const vec3& pos, const quaternion& ang, const mat4& projection);
	void set_view(const RenderParams& params, Camera* cam);
	void set_z(bool write, bool test);
	void set_wire(bool enabled);
	void set_cull(CullMode mode);

	owned<UniformBuffer> ubo_light;
	LightMetaData light_meta_data;
	void update_light_ubo();

	void clear(const RenderParams& params, const Array<color>& colors, float z=-1);

	RenderData& start(const RenderParams& params, const mat4& matrix,
	                  Shader* shader, const Material& material, int pass_no,
	                  PrimitiveTopology top, VertexBuffer *vb);


	base::map<Material*, ShaderCache> multi_pass_shader_cache[4];
	// material as id!
	Shader* get_shader(Material* material, int pass_no, const string& vertex_shader_module, const string& geometry_shader_module);
};

#endif //Y_RENDERVIEWDATA_H
