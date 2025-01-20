//
// Created by Michael Ankele on 2025-01-20.
//

#include "ModeWorld.h"

#include <Session.h>
#include <lib/base/iter.h>
#include <view/MultiView.h>
#include "data/DataWorld.h"
#include "data/WorldObject.h"
#include "data/WorldTerrain.h"
#include <y/renderer/Renderer.h>
#include <y/renderer/world/geometry/RenderViewData.h>
#include <y/renderer/world/geometry/SceneView.h>
#include <y/renderer/base.h>
#include <y/renderer/path/RenderPath.h>
#include <y/helper/ResourceManager.h>
#include <y/world/Material.h>
#include <y/world/Camera.h>
#include <y/world/Light.h>
#include <y/y/Entity.h>
#include <y/world/Model.h>
#include <y/world/ModelManager.h>
#include <y/world/Terrain.h>
#include <y/graphics-impl.h>
#include <lib/os/msg.h>
#include <view/EdwardWindow.h>

class DataWorldRenderer : public Renderer {
public:
	ModeWorld* mode;
	DataWorld* data_world;
	RenderViewData rvd;
	Light* light;
	base::map<Material*, ShaderCache> multi_pass_shader_cache[4];
	// material as id!
	Shader* get_shader(Material* material, int pass_no, const string& vertex_shader_module, const string& geometry_shader_module) {
		if (!multi_pass_shader_cache[pass_no].contains(material))
			multi_pass_shader_cache[pass_no].set(material, {});
		auto& cache = multi_pass_shader_cache[pass_no][material];
		RenderPathType type = RenderPathType::Forward;
	//	if (is_shadow_pass())
	//		cache._prepare_shader_multi_pass(type, *material_shadow, vertex_shader_module, geometry_shader_module, pass_no);
	//	else
			cache._prepare_shader_multi_pass(type, *material, vertex_shader_module, geometry_shader_module, pass_no);
		return cache.get_shader(type);
	}

	Material* material_hover;

	explicit DataWorldRenderer(ModeWorld* m, SceneView* scene_view) : Renderer("world") {
		mode = m;
		data_world = mode->data;
		resource_manager = data_world->session->resource_manager;

		rvd.scene_view = scene_view;

		light = new Light(White, -1, -1);
		light->owner = new Entity;
		light->owner->ang = quaternion::rotation({1,0,0}, 0.5f);
		light->light.harshness = 0.5f;

		try {
			material_hover = resource_manager->load_material("");
			material_hover->albedo = {0.3f, 0,0,0};
			material_hover->metal = 0.0f;
			material_hover->roughness = 0.9f;
			material_hover->emission = Red;
			material_hover->textures = {tex_white};
			material_hover->pass0.cull_mode = 0;
			material_hover->pass0.mode = TransparencyMode::FUNCTIONS;
			material_hover->pass0.source = Alpha::SOURCE_ALPHA;
			material_hover->pass0.destination = Alpha::SOURCE_INV_ALPHA;
			material_hover->pass0.z_buffer = false;
		} catch(Exception& e) {
			msg_error(e.message());
		}
	}
	void draw(const RenderParams& params) override {
		auto cb = params.command_buffer;
		cb->clear(params.area, {data_world->meta_data.background_color}, 1.0);

	//	scene_view.choose_lights();
		{
			rvd.scene_view->lights.clear();
			rvd.scene_view->shadow_index = -1;
			//	if (l->allow_shadow)
			//		scene_view.shadow_index = scene_view.lights.num;
			rvd.scene_view->lights.add(light);
		}

		rvd.scene_view->cam->update_matrices(params.desired_aspect_ratio);
		rvd.set_projection_matrix(rvd.scene_view->cam->m_projection);
		rvd.set_view_matrix(rvd.scene_view->cam->m_view);
		rvd.update_lights();
		rvd.ubo.num_lights = rvd.scene_view->lights.num;
		rvd.ubo.shadow_index = rvd.scene_view->shadow_index;

		rvd.begin_draw();

		for (auto& t: data_world->terrains) {
			t.terrain->prepare_draw(rvd.scene_view->cam->owner->pos);
			auto material = t.terrain->material.get();
			auto vb = t.terrain->vertex_buffer.get();

			auto shader = get_shader(material, 0, t.terrain->vertex_shader_module, "");
			auto& rd = rvd.start(params,  mat4::translation(t.pos), shader, *material, 0, PrimitiveTopology::TRIANGLES, vb);
			cb->push_constant(0, 4, &t.terrain->texture_scale[0].x);
			cb->push_constant(4, 4, &t.terrain->texture_scale[1].x);
			rd.apply(params);
			cb->draw(vb);
		}

		for (auto& o: data_world->objects) {
			for (int k=0; k<o.object->mesh[0]->sub.num; k++) {
				auto m = o.object;
				auto material = m->material[k];
				auto vb = m->mesh[0]->sub[k].vertex_buffer;

				auto shader = get_shader(material, 0, m->_template->vertex_shader_module, "");
				auto& rd = rvd.start(params,  mat4::translation(o.pos) * mat4::rotation(o.ang), shader, *material, 0, PrimitiveTopology::TRIANGLES, vb);
				rd.apply(params);
				cb->draw(vb);
			}
		}

		// hover...
		if (mode->hover && (*mode->hover).type == MVD_WORLD_OBJECT) {
			auto& o = data_world->objects[(*mode->hover).index];
			for (int k=0; k<o.object->mesh[0]->sub.num; k++) {
				auto m = o.object;
				auto vb = m->mesh[0]->sub[k].vertex_buffer;

				auto shader = get_shader(material_hover, 0, m->_template->vertex_shader_module, "");
				auto& rd = rvd.start(params,  mat4::translation(o.pos) * mat4::rotation(o.ang), shader, *material_hover, 0, PrimitiveTopology::TRIANGLES, vb);
				rd.apply(params);
				cb->draw(vb);
			}
		}
	}
};

ModeWorld::ModeWorld(Session* session) : Mode(session) {
	multi_view = new MultiView(session);
	data = new DataWorld(session);
}


Renderer* ModeWorld::create_renderer(SceneView* scene_view) {
	return new DataWorldRenderer(this, scene_view);
}

void ModeWorld::optimize_view() {
	vec3 vmin, vmax;
	data->get_bounding_box(vmin, vmax);
	multi_view->view_port.pos = (vmin + vmax) / 2;
	multi_view->view_port.radius = (vmax - vmin).length() * 0.7f;
	multi_view->view_port.ang = quaternion::rotation({0.35f, 0, 0});
}

#define MODEL_MAX_VERTICES	65536
vec3 tmv[MODEL_MAX_VERTICES*5],pmv[MODEL_MAX_VERTICES*5];
bool tvm[MODEL_MAX_VERTICES*5];


float model_hover_z(Model *o, const vector &pos, const vector &ang, const mat4& proj, const vec2 &mv, vec3 &tp) {
	if (!o)
		return -1;
	int d = 0;//o->_detail_;
	if ((d<0) or (d>2))
		return -1;
	o->_matrix = mat4::translation(pos) * mat4::rotation(ang);
	for (int i=0;i<o->mesh[d]->vertex.num;i++) {
		tmv[i] = o->_matrix * o->mesh[d]->vertex[i];
		pmv[i] = proj.project(tmv[i]);
	}
	float z_min = 1;
	for (int mm=0;mm<o->material.num;mm++)
		for (int i=0;i<o->mesh[d]->sub[mm].num_triangles;i++) {
			int ia = o->mesh[d]->sub[mm].triangle_index[i*3  ];
			int ib = o->mesh[d]->sub[mm].triangle_index[i*3+1];
			int ic = o->mesh[d]->sub[mm].triangle_index[i*3+2];
			vec3 a = pmv[ia];
			vec3 b = pmv[ib];
			vec3 c = pmv[ic];
			if ((a.z<=0) or (b.z<=0) or (c.z<=0) or (a.z>=1) or (b.z>=1) or (c.z>=1))
				continue;
			float az = a.z, bz = b.z, cz = c.z;
			a.z = b.z = c.z = 0;
			auto fg = bary_centric(vec3(mv.x,mv.y,0),a,b,c);
			if ((fg.x>=0) and (fg.y>=0) and (fg.x+fg.y<=1)) {
				float z = az + fg.x*(bz-az) + fg.y*(cz-az);
				if (z < z_min) {
					z_min = z;
					tp = tmv[ia] + fg.x*(tmv[ib]-tmv[ia]) + fg.y*(tmv[ic]-tmv[ia]);
				}
			}
		}
	return z_min;
}

float object_hover_distance(const WorldObject& me, const mat4& proj, const vec2 &mv, vec3 &tp, float &z) {
	Model *o = me.object;
	if (!o)
		return -1;
	z = model_hover_z(o, me.pos, me.ang, proj, mv, tp);
	return (z < 1) ? 0 : -1;
}

void ModeWorld::on_mouse_move(const vec2& m) {
	hover = base::None;

	vec3 tp;
	float zmin = multi_view->view_port.radius * 2;
	for (const auto& [i, o]: enumerate(data->objects)) {
		float z;
		float dist = object_hover_distance(o, multi_view->projection, m, tp, z);
		if (dist >= 0 and z < zmin) {
			zmin = z;
			hover = {MVD_WORLD_OBJECT, i};
		}
	}
}

void ModeWorld::on_draw_post(Painter* p) {


	p->set_color(Black);
	//p->set_font_size(20);
	//p->draw_str({100, 100}, str(r));

	p->set_color(Red);
	for (auto& o: data->objects) {
		auto p1 = multi_view->projection.project(o.pos);
		p->draw_rect({p1.x,p1.x+2, p1.y,p1.y+2});
	}
}




