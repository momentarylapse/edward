/*
 * GeometryRendererGL.cpp
 *
 *  Created on: Dec 15, 2022
 *      Author: michi
 */

#include "GeometryRenderer.h"

#ifdef USING_OPENGL
#include "RenderViewData.h"
#include "SceneView.h"
#include "../../base.h"
#include "../../../helper/PerformanceMonitor.h"
#include "../../../world/Material.h"
#include "../../../Config.h"
#include "../../../helper/PerformanceMonitor.h"
#include "../../../helper/ResourceManager.h"
#include "../../../plugins/PluginManager.h"
#include "../../../fx/Particle.h"
#include "../../../fx/Beam.h"
#include "../../../fx/ParticleManager.h"
#include "../../../fx/ParticleEmitter.h"
#include "../../../gui/gui.h"
#include "../../../gui/Picture.h"
#include "../../../world/Camera.h"
#include "../../../world/Material.h"
#include "../../../world/Model.h"
#include "../../../world/Terrain.h"
#include "../../../world/World.h"
#include "../../../world/Light.h"
#include "../../../world/ModelManager.h"
#include "../../../world/components/Animator.h"
#include "../../../world/components/UserMesh.h"
#include "../../../world/components/MultiInstance.h"
#include "../../../y/Entity.h"
#include "../../../y/ComponentManager.h"
#include "../../../meta.h"
#include <lib/base/sort.h>
#include <lib/nix/nix.h>
#include <lib/image/image.h>
#include <lib/math/vec3.h>
#include <lib/math/complex.h>
#include <lib/math/rect.h>
#include <lib/os/msg.h>



#if 0
void GeometryRendererGL::set_textures(const Array<Texture*> &tex) {
	auto tt = tex;
	if (tt.num == 0)
		tt.add(tex_white);
	if (tt.num == 1)
		tt.add(tex_white);
	if (tt.num == 2)
		tt.add(tex_white);
	/*tt.add(fb_shadow1->depth_buffer.get());
	tt.add(fb_shadow2->depth_buffer.get());
	tt.add(cube_map.get());*/
	nix::bind_textures(tt);
}
#endif

void create_color_quad(VertexBuffer *vb, const rect &d, const rect &s, const color &c) {
	Array<VertexFx> v = {{{d.x1,d.y1,0}, c, s.x1,s.y1},
	                    {{d.x1,d.y2,0}, c, s.x1,s.y2},
	                    {{d.x2,d.y2,0}, c, s.x2,s.y2},
	                    {{d.x1,d.y1,0}, c, s.x1,s.y1},
	                    {{d.x2,d.y2,0}, c, s.x2,s.y2},
	                    {{d.x2,d.y1,0}, c, s.x2,s.y1}};
	vb->update(v);
}


void GeometryRenderer::draw_particles(const RenderParams& params, RenderViewData &rvd) {
	PerformanceMonitor::begin(ch_fx);
	gpu_timestamp_begin(params, ch_fx);

	nix::set_shader(shader_fx.get());
	nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	nix::set_z(false, true);
	nix::set_cull(nix::CullMode::NONE);

	auto cam = scene_view.cam;

	auto r = mat4::rotation(cam->owner->ang);

	auto add_beam_vertices = [&cam] (Array<VertexFx>& v, const vec3& pos, const vec3& length, float radius, const color& col, const rect& source) {
		// TODO geometry shader!
		auto pa = cam->project(pos);
		auto pb = cam->project(pos + length);
		auto pe = vec3::cross(pb - pa, vec3::EZ).normalized();
		auto uae = cam->unproject(pa + pe * 0.1f);
		auto ube = cam->unproject(pb + pe * 0.1f);
		auto _e1 = (pos - uae).normalized() * radius;
		auto _e2 = (pos + length - ube).normalized() * radius;
		//vec3 e1 = -vec3::cross(cam->ang * vec3::EZ, p.length).normalized() * p.radius/2;

		vec3 p00 = pos - _e1;
		vec3 p01 = pos - _e2 + length;
		vec3 p10 = pos + _e1;
		vec3 p11 = pos + _e2 + length;

		v.add({p00, col, source.x1, source.y1});
		v.add({p01, col, source.x2, source.y1});
		v.add({p11, col, source.x2, source.y2});
		v.add({p00, col, source.x1, source.y1});
		v.add({p11, col, source.x2, source.y2});
		v.add({p10, col, source.x1, source.y2});
	};

	// legacy particles
	base::map<Texture*, Array<LegacyParticle*>> legacy_groups;
	auto& legacy_particles = ComponentManager::get_list_family<LegacyParticle>();
	for (auto p: legacy_particles) {
		int i = legacy_groups.find(p->texture.get());
		if (i >= 0) {
			legacy_groups.by_index(i).add(p);
		} else {
			legacy_groups.add({p->texture.get(), {p}});
		}
	}

	for (const auto& [texture, particles]: legacy_groups) {
		nix::bind_texture(0, texture);

		Array<VertexFx> v;
		for (auto p: particles)
			if (p->enabled) {
				if (p->is_beam) {
					auto b = reinterpret_cast<LegacyBeam*>(p);
					add_beam_vertices(v, p->owner->pos, b->length, p->radius, p->col, p->source);
				} else {
					auto m = mat4::translation(p->owner->pos) * r * mat4::scale(p->radius, p->radius, p->radius);

					v.add({m * vec3(-1, 1, 0), p->col, p->source.x1, p->source.y1});
					v.add({m * vec3(1, 1, 0), p->col, p->source.x2, p->source.y1});
					v.add({m * vec3(1, -1, 0), p->col, p->source.x2, p->source.y2});
					v.add({m * vec3(-1, 1, 0), p->col, p->source.x1, p->source.y1});
					v.add({m * vec3(1, -1, 0), p->col, p->source.x2, p->source.y2});
					v.add({m * vec3(-1, -1, 0), p->col, p->source.x1, p->source.y2});
				}
			}
		vb_fx->update(v);
		shader_fx->set_matrix_l(shader_fx->location[Shader::LOCATION_MATRIX_M], mat4::ID);
		nix::draw_triangles(vb_fx.get());
	}


	auto& particle_groups = ComponentManager::get_list_family<ParticleGroup>();
	for (auto g: particle_groups) {
		nix::bind_texture(0, g->texture);
		int count = 0;
		for (auto& p: g->particles)
			count += int(p.enabled);

		nix::set_shader(shader_fx_points.get());
		shader_fx_points->set_floats("source_uv", &g->source.x1, 4);
		Array<VertexPoint> v;
		v.__reserve(count);
		for (auto& p: g->particles)
			if (p.enabled)
				v.add({p.pos, p.radius*2, p.col});
		vb_fx_points->update(v);
		shader_fx->set_matrix_l(shader_fx->location[Shader::LOCATION_MATRIX_M], mat4::ID);
		nix::draw_points(vb_fx_points.get());
	}

	// beams
	//Array<Vertex1> v = {{v_0, v_0, 0,0}, {v_0, v_0, 0,1}, {v_0, v_0, 1,1}, {v_0, v_0, 0,0}, {v_0, v_0, 1,1}, {v_0, v_0, 1,0}};
	nix::set_shader(shader_fx.get());
	shader_fx->set_matrix_l(shader_fx->location[Shader::LOCATION_MATRIX_M], mat4::ID);
	for (auto g: particle_groups) {
		nix::bind_texture(0, g->texture);
		auto source = g->source;

		Array<VertexFx> v;

		for (auto& b: g->beams)
			if (b.enabled)
				add_beam_vertices(v, b.pos, b.length, b.radius, b.col, source);

		vb_fx->update(v);
		nix::draw_triangles(vb_fx.get());
	}


	nix::set_z(true, true);
	nix::disable_alpha();
	gpu_timestamp_end(params, ch_fx);
	PerformanceMonitor::end(ch_fx);
}

void GeometryRenderer::clear(const RenderParams& params, RenderViewData &rvd) {
	nix::clear_color(world.background);
	nix::clear_z();
}



#endif
