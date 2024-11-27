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
class RenderViewData;

enum class RenderPathType;
enum class ShaderVariant;



class GeometryRendererGL : public GeometryRenderer {
public:
	GeometryRendererGL(RenderPathType type, SceneView &scene_view);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override {}

	static void set_material(const SceneView& scene_view, ShaderCache& cache, const Material& m, RenderPathType type, const string& vertex_module, const string& geometry_module);
	static void set_material_x(const SceneView& scene_view, const Material& m, Shader* shader);

	void draw_skyboxes(const RenderParams& params, RenderViewData &rvd);
	void draw_particles(const RenderParams& params, RenderViewData &rvd);
	void draw_terrains(const RenderParams& params, RenderViewData &rvd);
	void draw_objects_opaque(const RenderParams& params, RenderViewData &rvd);
	void draw_objects_transparent(const RenderParams& params, RenderViewData &rvd);
	void draw_objects_instanced(const RenderParams& params, RenderViewData &rvd);
	void draw_user_meshes(const RenderParams& params, RenderViewData &rvd, bool transparent);
	void prepare_instanced_matrices();

	void draw_opaque(const RenderParams& params, RenderViewData &rvd);
	void draw_transparent(const RenderParams& params, RenderViewData &rvd);
};

#endif
