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

class GeometryRendererGL : public GeometryRenderer {
public:
	GeometryRendererGL(RenderPathType type, SceneView &scene_view);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override {}

	void set_material(ShaderCache &cache, Material *m, RenderPathType type, const string &vertex_module, const string &geometry_module);
	void set_material_x(Material *m, Shader *shader);

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
