/*
 * GeometryRendererVulkan.h
 *
 *  Created on: Dec 16, 2022
 *      Author: michi
 */

#pragma once

#include "GeometryRenderer.h"
#ifdef USING_VULKAN
#include <lib/math/mat4.h>

class Camera;
class PerformanceMonitor;
class Material;
struct ShaderCache;

enum class RenderPathType;
enum class ShaderVariant;

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

struct RenderDataVK {
	UniformBuffer* ubo;
	DescriptorSet* dset;
	void set_textures(const SceneView& scene_view, const Array<Texture*>& tex);
	void apply(const RenderParams& params);
};

struct RenderViewDataVK {
	RenderViewDataVK();

	owned<UniformBuffer> ubo_light;
	Array<RenderDataVK> rda;

	int index = 0;
	UBO ubo;
	SceneView* scene_view = nullptr;
	RenderDataVK& start(const RenderParams& params, RenderPathType type, const mat4& matrix,
			ShaderCache& shader_cache, const Material& material, int pass_no,
			const string& vertex_shader_module, const string& geometry_shader_module,
			PrimitiveTopology top, VertexBuffer *vb);
};

class GeometryRendererVulkan : public GeometryRenderer {
public:
	GeometryRendererVulkan(RenderPathType type, SceneView &scene_view);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override {}

	static GraphicsPipeline *get_pipeline(Shader *s, RenderPass *rp, const Material::RenderPassData &pass, PrimitiveTopology top, VertexBuffer *vb);

	void draw_particles(const RenderParams& params, RenderViewDataVK &rvd);
	void draw_skyboxes(const RenderParams& params, RenderViewDataVK &rvd);
	void draw_terrains(const RenderParams& params, RenderViewDataVK &rvd);
	void draw_objects_opaque(const RenderParams& params, RenderViewDataVK &rvd);
	void draw_objects_transparent(const RenderParams& params, RenderViewDataVK &rvd);
	void draw_objects_instanced(const RenderParams& params, RenderViewDataVK &rvd);
	void draw_user_meshes(const RenderParams& params, bool transparent, RenderViewDataVK &rvd);

	void prepare_instanced_matrices();
	void prepare_lights(Camera *cam, RenderViewDataVK &rvd);
};

#endif
