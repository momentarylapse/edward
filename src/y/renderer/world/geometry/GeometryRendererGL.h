/*
 * GeometryRendererGL.h
 *
 *  Created on: Dec 15, 2022
 *      Author: michi
 */

#pragma once

#include "GeometryRenderer.h"
#ifdef USING_OPENGL
#include <lib/math/mat4.h>
#include <lib/base/map.h>

class Camera;
class PerformanceMonitor;
class Material;
struct ShaderCache;

enum class RenderPathType;
enum class ShaderVariant;


struct RenderData {
	//UniformBuffer* ubo;
	void set_textures(const SceneView& scene_view, const Array<Texture*>& tex);
	void apply(const RenderParams& params);
};

struct RenderViewData {
	RenderViewData();
	void reset() {}

	struct UBO {
		int num_lights, shadow_index;
	} ubo;

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

class GeometryRendererGL : public GeometryRenderer {
public:
	GeometryRendererGL(RenderPathType type, SceneView &scene_view);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override {}

	static void set_material(const SceneView& scene_view, ShaderCache& cache, const Material& m, RenderPathType type, const string& vertex_module, const string& geometry_module);
	static void set_material_x(const SceneView& scene_view, const Material& m, Shader* shader);

	void draw_skyboxes();
	void draw_particles();
	void draw_terrains();
	void draw_objects_opaque();
	void draw_objects_transparent(const RenderParams& params);
	void draw_objects_instanced();
	void draw_user_meshes(bool transparent);
	void prepare_instanced_matrices();

	void draw_opaque();
	void draw_transparent(const RenderParams& params);
};

#endif
