//
// Created by Michael Ankele on 2025-05-07.
//

#pragma once

#include <lib/yrenderer/scene/MeshEmitter.h>
#include <lib/ygraphics/graphics-fwd.h>
#include <lib/yrenderer/Material.h>
#include "lib/math/vec3.h"


struct VertexFx {
	vec3 pos;
	color col;
	float u, v;
};

struct VertexPoint {
	vec3 pos;
	float radius;
	color col;
};

struct Camera;


class WorldParticlesEmitter : public yrenderer::MeshEmitter {
public:
	explicit WorldParticlesEmitter(yrenderer::Context* ctx, Camera* cam);
	void emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) override;

	void draw_legacy_groups(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, ygfx::Shader* shader, const mat4& r);
	void draw_particle_groups(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, ygfx::Shader* shader, const mat4& r);
	void draw_beam_groups(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, ygfx::Shader* shader, const mat4& r);

	int current_vb = 0;
	ygfx::VertexBuffer* get_vb();

	Camera* cam;

//	shared<Shader> shader_fx;
//	shared<Shader> shader_fx_points;
	owned<ygfx::VertexBuffer> vb_fx;
	owned<ygfx::VertexBuffer> vb_fx_points;

	owned<yrenderer::Material> fx_material;

	owned_array<ygfx::VertexBuffer> fx_vertex_buffers;
};

