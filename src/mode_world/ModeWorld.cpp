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
#include "data/WorldCamera.h"
#include "data/WorldLight.h"
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
#include <view/DrawingHelper.h>

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
			if (o.is_selected)
				for (int k=0; k<o.object->mesh[0]->sub.num; k++) {
					auto m = o.object;
					auto vb = m->mesh[0]->sub[k].vertex_buffer;
					draw_mesh(params, mat4::translation(o.pos) * mat4::rotation(o.ang), vb, material_selection, m->_template->vertex_shader_module);
				}
		}

		// hover...
		if (mode->multi_view->hover and mode->multi_view->hover->type == MultiViewType::WORLD_OBJECT) {
			auto& o = data_world->objects[mode->multi_view->hover->index];
			for (int k=0; k<o.object->mesh[0]->sub.num; k++) {
				auto m = o.object;
				auto vb = m->mesh[0]->sub[k].vertex_buffer;
				draw_mesh(params, mat4::translation(o.pos) * mat4::rotation(o.ang), vb, material_hover, m->_template->vertex_shader_module);
			}
		}


		draw_cameras();
		draw_lights(mode->multi_view->active_window);

		mode->multi_view->action_controller->draw(params, rvd);
	}

	void draw_cameras() {
		for (auto &c: data_world->cameras) {
			//if (c.view_stage < mode->multi_view->view_stage)
			//	continue;

			auto dh = mode->session->drawing_helper;
			auto win = dh->window;

			dh->set_color(color(1, 0.9f, 0.6f, 0.3f));
			dh->set_line_width(3);//scheme.LINE_WIDTH_THIN);
			if (c.is_selected) {
				dh->set_color(Red);
				dh->set_line_width(5);//scheme.LINE_WIDTH_MEDIUM);
			}
			auto q = quaternion::rotation_v(c.ang);
			float r = win->multi_view->view_port.radius * 0.1f;
			float rr = r * tan(c.fov / 2);
			vec3 ex = q * vec3::EX * rr * 1.333f;
			vec3 ey = q * vec3::EY * rr;
			vec3 ez = q * vec3::EZ * r;

			Array<vec3> points = {
				c.pos, c.pos + ez + ex + ey,
				c.pos, c.pos + ez - ex + ey,
				c.pos, c.pos + ez + ex - ey,
				c.pos, c.pos + ez - ex - ey,
				c.pos + ez + ex + ey, c.pos + ez - ex + ey,
				c.pos + ez - ex + ey, c.pos + ez - ex - ey,
				c.pos + ez - ex - ey, c.pos + ez + ex - ey,
				c.pos + ez + ex - ey, c.pos + ez + ex + ey};
			dh->draw_lines(points, false);
		}
	}



	void draw_tangent_circle(MultiViewWindow *win, const vec3 &p, const vec3 &c, const vec3 &n, float r) {

		vec3 e1 = n.ortho();
		vec3 e2 = n ^ e1;
		e1 *= r;
		e2 *= r;
		vec2 pc = win->project(c).xy();
		int N = 64;
		int i_max = 0;
		float d_max = 0;
		for (int i=0; i<=N; i++) {
			float w = i * 2 * pi / N;
			vec2 pp = win->project(c + sin(w) * e1 + cos(w) * e2).xy();
			if ((pp - pc).length() > d_max) {
				i_max = i;
				d_max = (pp - pc).length();
			}
		}
		float w = i_max * 2 * pi / N;
		auto dh = mode->session->drawing_helper;
		dh->draw_lines({p, c + sin(w) * e1 + cos(w) * e2,
			p, c - sin(w) * e1 - cos(w) * e2}, false);
	}

	const float LIGHT_RADIUS_FACTOR_HI = 0.03f;
	const float LIGHT_RADIUS_FACTOR_LO = 0.15f;

	void draw_lights(MultiViewWindow *win) {
		auto dh = mode->session->drawing_helper;
		for (auto &l: data_world->lights) {
			//if (l.view_stage < multi_view->view_stage)
			//	continue;

			dh->set_color(color(1, 0.9f, 0.6f, 0.3f));
			dh->set_line_width(5);//scheme.LINE_WIDTH_MEDIUM);
			if (l.is_selected) {
				dh->set_color(Red);
				dh->set_line_width(7);//scheme.LINE_WIDTH_THICK);
			}

			if (l.type == LightType::DIRECTIONAL) {
				dh->draw_lines({l.pos, l.pos + l.ang.ang2dir() * win->multi_view->view_port.radius * 0.1f}, false);
			} else if (l.type == LightType::POINT) {
				//draw_circle(l.pos, win->get_direction(), l.radius);
				dh->draw_circle(l.pos, win->dir(), l.radius * LIGHT_RADIUS_FACTOR_LO);
				dh->draw_circle(l.pos, win->dir(), l.radius * LIGHT_RADIUS_FACTOR_HI);
			} else if (l.type == LightType::CONE) {
				dh->draw_lines({l.pos, l.pos + l.ang.ang2dir() * l.radius * LIGHT_RADIUS_FACTOR_LO}, false);
				dh->draw_circle(l.pos + l.ang.ang2dir() * l.radius*LIGHT_RADIUS_FACTOR_LO, l.ang.ang2dir(), l.radius * tan(l.theta) * LIGHT_RADIUS_FACTOR_LO);
				dh->draw_circle(l.pos + l.ang.ang2dir() * l.radius*LIGHT_RADIUS_FACTOR_HI, l.ang.ang2dir(), l.radius * tan(l.theta) * LIGHT_RADIUS_FACTOR_HI);
				draw_tangent_circle(win, l.pos, l.pos + l.ang.ang2dir() * l.radius*LIGHT_RADIUS_FACTOR_LO, l.ang.ang2dir(), l.radius * tan(l.theta) * LIGHT_RADIUS_FACTOR_LO);
			}
		}
	}

};

ModeWorld::ModeWorld(Session* session) :
	Mode(session),
	temp(session)
{
	multi_view = new MultiView(session);
	data = new DataWorld(session);
	generic_data = data;
}


Renderer* ModeWorld::create_renderer(SceneView* scene_view) {
	return new DataWorldRenderer(this, scene_view);
}

void ModeWorld::on_enter() {
	multi_view->f_hover = [this] (MultiViewWindow* win, const vec2& m) {
		return get_hover(win, m);
	};
	multi_view->f_create_action = [this] {
		return new ActionWorldMoveSelection(data, data->get_selection());
	};
	multi_view->data_sets = {
		{MultiViewType::WORLD_OBJECT, &data->objects},
		{MultiViewType::WORLD_TERRAIN, &data->terrains},
		{MultiViewType::WORLD_CAMERA, &data->cameras},
		{MultiViewType::WORLD_LIGHT, &data->lights}
	};
}


void ModeWorld::optimize_view() {
	vec3 vmin, vmax;
	data->get_bounding_box(vmin, vmax);
	multi_view->view_port.suggest_for_box(vmin, vmax);
}

#define MODEL_MAX_VERTICES	65536
vec3 tmv[MODEL_MAX_VERTICES*5],pmv[MODEL_MAX_VERTICES*5];
bool tvm[MODEL_MAX_VERTICES*5];


float model_hover_z(Model *o, const vector &pos, const vector &ang, MultiViewWindow* win, const vec2 &mv, vec3 &tp) {
	if (!o)
		return -1;
	int d = 0;//o->_detail_;
	if ((d<0) or (d>2))
		return -1;
	o->_matrix = mat4::translation(pos) * mat4::rotation(ang);
	for (int i=0;i<o->mesh[d]->vertex.num;i++) {
		tmv[i] = o->_matrix * o->mesh[d]->vertex[i];
		pmv[i] = win->project(tmv[i]);
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

float object_hover_distance(const WorldObject& me, MultiViewWindow* win, const vec2 &mv, vec3 &tp, float &z) {
	Model *o = me.object;
	if (!o)
		return -1;
	z = model_hover_z(o, me.pos, me.ang, win, mv, tp);
	return (z < 1) ? 0 : -1;
}

base::optional<Hover> ModeWorld::get_hover(MultiViewWindow* win, const vec2& m) const {
	base::optional<Hover> h;

	float zmin = multi_view->view_port.radius * 2;
	for (const auto& [i, o]: enumerate(data->objects)) {
		float z;
		vec3 tp;
		float dist = object_hover_distance(o, win, m, tp, z);
		if (dist >= 0 and z < zmin) {
			zmin = z;
			h = {MultiViewType::WORLD_OBJECT, i, tp};
		}
	}
	return h;
}

Data::Selection ModeWorld::get_selection(MultiViewWindow* win, const rect& _r) const {
	auto r = _r.canonical();
	Data::Selection s;
	s.add({MultiViewType::WORLD_OBJECT, {}});
	for (const auto& [i, o]: enumerate(data->objects)) {
		const auto p = win->project(o.pos);
		if (p.z <= 0 or p.z >= 1)
			continue;
		if (r.inside({p.x, p.y}))
			s[MultiViewType::WORLD_OBJECT].add(i);
	}
	return s;
}

void ModeWorld::on_mouse_move(const vec2& m, const vec2& d) {
	out_redraw();
}

void ModeWorld::on_mouse_leave(const vec2& m) {
	multi_view->hover = base::None;
	out_redraw();
}

void ModeWorld::on_left_button_down(const vec2& m) {
	out_redraw();
}

void ModeWorld::on_left_button_up(const vec2&) {
	out_redraw();
}

static base::optional<string> world_selection_description(DataWorld* data) {
	int nob = 0, nter = 0, ncam = 0, nlights = 0;
	auto sel = data->get_selection();
	if (sel.contains(MultiViewType::WORLD_OBJECT))
		nob = sel[MultiViewType::WORLD_OBJECT].num;
	if (sel.contains(MultiViewType::WORLD_TERRAIN))
		nter = sel[MultiViewType::WORLD_TERRAIN].num;
	if (sel.contains(MultiViewType::WORLD_CAMERA))
		ncam = sel[MultiViewType::WORLD_CAMERA].num;
	if (sel.contains(MultiViewType::WORLD_LIGHT))
		nlights = sel[MultiViewType::WORLD_LIGHT].num;
	if (nob + nter + ncam + nlights == 0)
		return base::None;
	Array<string> s;
	if (nob > 0)
		s.add(format("%d objects", nob));
	if (nter > 0)
		s.add(format("%d terrains", nter));
	if (ncam > 0)
		s.add(format("%d cameras", ncam));
	if (nlights > 0)
		s.add(format("%d lights", nlights));
	return implode(s, ", ");
}

void ModeWorld::on_draw_post(Painter* p) {
	p->set_color(Black);
	//p->set_font_size(20);
	//p->draw_str({100, 100}, str(r));

	p->set_color(Red);
	for (auto& o: data->objects) {
		auto p1 = multi_view->active_window->project(o.pos);
		p->draw_rect({p1.x-2,p1.x+2, p1.y-2,p1.y+2});
	}
	p->set_color(Blue);
	for (auto& o: data->lights) {
		auto p1 = multi_view->active_window->project(o.pos);
		p->draw_rect({p1.x-2,p1.x+2, p1.y-2,p1.y+2});
	}

	p->set_color(White);
	if (auto s = world_selection_description(data))
		p->draw_str(p->area().p01() + vec2(30, -40), "selected: " + *s);
}

void ModeWorld::on_command(const string& id) {
	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();
}

void ModeWorld::on_key_down(int key) {
	if (key == xhui::KEY_DELETE or key == xhui::KEY_BACKSPACE) {
		if (auto s = world_selection_description(data)) {
			data->delete_selection(data->get_selection());
			session->set_message("deleted: " + *s);
		} else {
			session->set_message("nothing selected");
		}
	}
	if (key == xhui::KEY_CONTROL + xhui::KEY_C) {
		data->copy(temp);
		if (temp.is_empty())
			session->set_message("nothing selected");
		else
			session->set_message("copied: " + *world_selection_description(data));
	}
	if (key == xhui::KEY_CONTROL + xhui::KEY_V) {
		multi_view->clear_selection();
		if (temp.is_empty()) {
			session->set_message("nothing to paste");
		} else {
			data->paste(temp);
			session->set_message("pasted: " + *world_selection_description(data));
		}
	}
}






