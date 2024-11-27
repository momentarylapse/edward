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
class RenderViewData;

class GeometryRendererVulkan : public GeometryRenderer {
public:
	GeometryRendererVulkan(RenderPathType type, SceneView &scene_view);

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override {}

	static GraphicsPipeline *get_pipeline(Shader *s, RenderPass *rp, const Material::RenderPassData &pass, PrimitiveTopology top, VertexBuffer *vb);

	void draw_particles(const RenderParams& params, RenderViewData &rvd);
	void draw_skyboxes(const RenderParams& params, RenderViewData &rvd);
	void draw_terrains(const RenderParams& params, RenderViewData &rvd);
	void draw_objects_opaque(const RenderParams& params, RenderViewData &rvd);
	void draw_objects_transparent(const RenderParams& params, RenderViewData &rvd);
	void draw_objects_instanced(const RenderParams& params, RenderViewData &rvd);
	void draw_user_meshes(const RenderParams& params, bool transparent, RenderViewData &rvd);

	void prepare_instanced_matrices();
	void prepare_lights(Camera *cam, RenderViewData &rvd);
};

#endif
