//
// Created by Michael Ankele on 2025-01-20.
//

#include "ModeWorld.h"
#include "action/ActionWorldMoveSelection.h"
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

#include <data/geometry/GeometryTeapot.h>
#include <view/ActionController.h>

Material* create_material(ResourceManager* resource_manager, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false) {
	auto material = resource_manager->load_material("");
	material->albedo = albedo;
	material->roughness = roughness;
	material->metal = metal;
	material->emission = emission;
	material->textures = {tex_white};
	if (transparent) {
		material->pass0.cull_mode = 0;
		material->pass0.mode = TransparencyMode::FUNCTIONS;
		material->pass0.source = Alpha::SOURCE_ALPHA;
		material->pass0.destination = Alpha::SOURCE_INV_ALPHA;
		material->pass0.z_buffer = false;
	}
	return material;
}

class DataWorldRenderer : public Renderer {
public:
	ModeWorld* mode;
	DataWorld* data_world;
	RenderViewData rvd;
	Light* light;

	void draw_mesh(const RenderParams& params, const mat4& matrix, VertexBuffer* vb, Material* material, const string& vertex_module = "default") {
		auto shader = rvd.get_shader(material, 0, vertex_module, "");
		auto& rd = rvd.start(params, matrix, shader, *material, 0, PrimitiveTopology::TRIANGLES, vb);
		rd.apply(params);
		params.command_buffer->draw(vb);
	}

	Material* material_hover;
	Material* material_selection;

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
			material_hover = create_material(resource_manager, {0.3f, 0,0,0}, 0.9f, 0, White, true);
			material_selection = create_material(resource_manager, {0.3f, 0,0,0}, 0.9f, 0, Red, true);
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

			auto shader = rvd.get_shader(material, 0, t.terrain->vertex_shader_module, "");
			auto& rd = rvd.start(params, mat4::translation(t.pos), shader, *material, 0, PrimitiveTopology::TRIANGLES, vb);
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
				draw_mesh(params, mat4::translation(o.pos) * mat4::rotation(o.ang), vb, material, m->_template->vertex_shader_module);
			}
		}

		// selection
		for (auto& o: data_world->objects) {
			if (mode->selection.contains(&o))
				for (int k=0; k<o.object->mesh[0]->sub.num; k++) {
					auto m = o.object;
					auto vb = m->mesh[0]->sub[k].vertex_buffer;
					draw_mesh(params, mat4::translation(o.pos) * mat4::rotation(o.ang), vb, material_selection, m->_template->vertex_shader_module);
				}
		}

		// hover...
		if (mode->hover and mode->hover->type == MVD_WORLD_OBJECT) {
			auto& o = data_world->objects[mode->hover->index];
			for (int k=0; k<o.object->mesh[0]->sub.num; k++) {
				auto m = o.object;
				auto vb = m->mesh[0]->sub[k].vertex_buffer;
				draw_mesh(params, mat4::translation(o.pos) * mat4::rotation(o.ang), vb, material_hover, m->_template->vertex_shader_module);
			}
		}

		mode->multi_view->action_controller->draw(params, rvd);
	}

};

ModeWorld::ModeWorld(Session* session) : Mode(session) {
	multi_view = new MultiView(session);
	data = new DataWorld(session);
	generic_data = data;
}


Renderer* ModeWorld::create_renderer(SceneView* scene_view) {
	return new DataWorldRenderer(this, scene_view);
}

void ModeWorld::optimize_view() {
	vec3 vmin, vmax;
	data->get_bounding_box(vmin, vmax);
	multi_view->view_port.suggest_for_box(vmin, vmax);
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

base::optional<ModeWorld::Hover> ModeWorld::get_hover(const vec2& m) const {
	base::optional<Hover> h;
	vec3 tp;
	float zmin = multi_view->view_port.radius * 2;
	for (const auto& [i, o]: enumerate(data->objects)) {
		float z;
		float dist = object_hover_distance(o, multi_view->projection, m, tp, z);
		if (dist >= 0 and z < zmin) {
			zmin = z;
			h = {MVD_WORLD_OBJECT, i};
		}
	}
	return h;
}

ModeWorld::Selection ModeWorld::get_selection(const rect& _r) const {
	auto r = _r.canonical();
	Selection s;
	for (const auto& [i, o]: enumerate(data->objects)) {
		const auto p = multi_view->projection.project(o.pos);
		if (p.z <= 0 or p.z >= 1)
			continue;
		if (r.inside({p.x, p.y}))
			s.add(&o);
	}
	return s;
}

void ModeWorld::update_selection_box() {
	if (selection.num > 0) {
		Box box;
		bool first = true;
		for (const auto& o: data->objects)
			if (selection.contains((void*)&o)) {
				if (first) {
					box = {o.pos + o.object->prop.min, o.pos + o.object->prop.max};
					first = false;
				} else {
					box.min._min(o.pos + o.object->prop.min);
					box.max._max(o.pos + o.object->prop.max);
				}
			}
		multi_view->set_selection_box(box);
	} else {
		multi_view->set_selection_box(base::None);
	}
}


void ModeWorld::on_mouse_move(const vec2& m, const vec2& d) {
	if (multi_view->action) {
		multi_view->action_trafo = multi_view->action_trafo * mat4::translation({d.x, d.y, 0});
		multi_view->action->update_and_notify(data, multi_view->action_trafo);
	} else if (multi_view->selection_area) {
		multi_view->selection_area = rect(multi_view->selection_area->p00(), m);
		auto s = get_selection(*multi_view->selection_area);
		// TODO shift/control
		if (s != selection) {
			selection = s;
			update_selection_box();
		}
	} else if (session->win->button(0)) {
		// start selection rect
		multi_view->selection_area = rect(m - d, m);
		//update_selection_box();
	} else {
		hover = get_hover(m);
	}
	out_redraw();
}

void ModeWorld::on_mouse_leave(const vec2& m) {
	hover = base::None;
	out_redraw();
}

void ModeWorld::on_left_button_down(const vec2&) {
	if (hover) {
		void* p = nullptr;
		if (hover->type == MVD_WORLD_OBJECT)
			p = &data->objects[hover->index];

		if (session->win->is_key_pressed(xhui::KEY_SHIFT)) {
			if (selection.contains(p))
				selection.erase(p);
			else
				selection.add(p);
			update_selection_box();
		} else if (session->win->is_key_pressed(xhui::KEY_CONTROL)) {
			selection.add(p);
			update_selection_box();
		} else {

			if (selection.contains(p)) {
				multi_view->action = new ActionWorldMoveSelection(data, selection);
				multi_view->action_trafo = mat4::ID;
			} else {
				selection = {p};
				update_selection_box();
			}
		}
	} else {
		if (!session->win->is_key_pressed(xhui::KEY_SHIFT) and !session->win->is_key_pressed(xhui::KEY_CONTROL)) {
			selection.clear();
			update_selection_box();
		}
	}
	out_redraw();
}

void ModeWorld::on_left_button_up(const vec2&) {
	if (multi_view->action) {
		data->execute(multi_view->action);
		multi_view->action = nullptr;
	}
	if (multi_view->selection_area)
		multi_view->selection_area = base::None;
	out_redraw();
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

	if (multi_view->selection_area) {
		p->set_color({0.2, 0,0,1});
		p->draw_rect(multi_view->selection_area->canonical());
		p->set_fill(false);
		p->set_color(Blue);
		p->set_line_width(2);
		p->draw_rect(multi_view->selection_area->canonical());
		p->set_fill(true);
	}
}

void ModeWorld::on_command(const string& id) {
	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();
}

void ModeWorld::on_key_down(int key) {
	if (key == xhui::KEY_DELETE or key == xhui::KEY_BACKSPACE) {
		data->delete_selection(selection);
		selection.clear();
		update_selection_box();
		hover = base::None;
	}
}






