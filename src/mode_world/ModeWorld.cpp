//
// Created by Michael Ankele on 2025-01-20.
//

#include "ModeWorld.h"
#include "ModeAddEntity.h"
#include "EntityPanel.h"
#include "action/ActionWorldMoveSelection.h"
#include <Session.h>
#include <lib/base/iter.h>
#include <lib/kaba/kaba.h>
#include <lib/os/filesystem.h>
#include <view/MultiView.h>
#include "data/DataWorld.h"
#include "data/WorldObject.h"
#include "data/WorldTerrain.h"
#include "data/WorldCamera.h"
#include "data/WorldLight.h"
#include <y/renderer/Renderer.h>
#include <y/renderer/world/geometry/RenderViewData.h>
#include <y/renderer/world/geometry/SceneView.h>
#include <y/helper/ResourceManager.h>
#include <y/world/Material.h>
#include <y/world/Camera.h>
#include <y/world/Light.h>
#include <y/y/Entity.h>
#include <y/world/Model.h>
#include <y/world/ModelManager.h>
#include <y/world/Terrain.h>
#include <y/world/World.h>
#include <y/graphics-impl.h>
#include <lib/os/msg.h>
#include <lib/os/terminal.h>
#include <lib/xhui/config.h>
#include <lib/xhui/controls/Toolbar.h>
#include <storage/Storage.h>
#include <view/EdwardWindow.h>
#include <view/ActionController.h>
#include <view/DrawingHelper.h>
#include "dialog/PropertiesDialog.h"


ModeWorld::ModeWorld(Session* session) :
	Mode(session),
	temp(session)
{
	multi_view = new MultiView(session);
	data = new DataWorld(session);
	generic_data = data;

	data->out_changed >> create_sink([this] {
		//data->lights
		//	world.en
	});
}


// seems quick enough
Array<ScriptInstanceData> enumerate_classes(Session *session, const string& full_base_class) {
	string base_class = full_base_class.explode(".").back();
	Array<ScriptInstanceData> r;
	auto files = os::fs::search(session->storage->root_dir_kind[FD_SCRIPT], "*.kaba", "rf");
	for (auto &f: files) {
		try {
			auto context = ownify(kaba::Context::create());
			auto s = context->load_module(session->storage->root_dir_kind[FD_SCRIPT] | f, true);
			for (auto c: s->classes()) {
				if (c->is_derived_from_s(full_base_class) and c->name != base_class)
					r.add({f, c->name});
			}
		} catch (Exception &e) {
			msg_error(e.message());
		}
	}
	return r;
}

void ModeWorld::on_enter() {
	multi_view->f_hover = [this] (MultiViewWindow* win, const vec2& m) {
		return get_hover(win, m);
	};
	multi_view->f_create_action = [this] {
		return new ActionWorldMoveSelection(data, data->get_selection());
	};
	multi_view->data_sets = {
		{MultiViewType::WORLD_ENTITY, &data->entities}
	};

	auto tb = session->win->toolbar;
	tb->set_by_id("world-toolbar");

	session->win->event("properties", [this] {
		session->win->open_dialog(new PropertiesDialog(session->win, data));
	});
	session->win->event("run-game", [this] {
		Path engine_dir = xhui::config.get_str("EngineDir", "");
		if (engine_dir.is_empty()) {
			session->error("cn not run egine. Config 'EngineDir' is not set");
			return;
		}

		auto cmd = format("cd \"%s\"; \"%s\" \"%s\"", session->storage->root_dir, engine_dir | "y", data->filename.basename_no_ext());
		try {
			os::terminal::shell_execute(cmd);
		} catch (Exception &e) {
			session->error(format("failed to run '%s'", cmd));
		}
	});

	session->win->event("add-entity", [this] {
		//session->set_message("add entity");
		//session->set_mode(new ModeAddEntity(this));
	});

	session->win->event_x("area", xhui::event_id::DragDrop, [this] {
		const vec3 p = multi_view->cursor_pos_3d(session->win->drag.m);
		if (session->win->drag.payload.match("add-entity-default-*")) {
			int index = session->win->drag.payload.tail(1)._int();
			WorldEntity e;
			e.pos = p;
			if (index == 0) {
				e.basic_type = MultiViewType::WORLD_ENTITY;
			} else if (index == 1) {
				e.basic_type = MultiViewType::WORLD_CAMERA;
				e.camera.min_depth = 1;
				e.camera.max_depth = 100000;
				e.camera.fov = 0.7f;
				e.camera.exposure = 1;
			} else {
				e.basic_type = MultiViewType::WORLD_LIGHT;
				e.light.col = White;
				e.light.type = LightType::DIRECTIONAL;
				e.light.radius = 0;
				e.light.theta = 0;
				if (index == 3) {
					e.light.type = LightType::POINT;
					e.light.radius = multi_view->view_port.radius * 0.3f;
				} else if (index == 4) {
					e.light.type = LightType::CONE;
					e.light.radius = multi_view->view_port.radius * 0.3f;
					e.light.theta = 0.5f;
				}
				e.light.harshness = 1;
				e.light.enabled = true;
			}
			data->add_entity(e);
		} else if (session->win->drag.payload.match("filename:*.model")) {
			Path filename = session->win->drag.payload.sub_ref(9);
			auto fn_rel = filename.relative_to(session->storage->get_root_dir(FD_MODEL));
			session->set_message(str(fn_rel));
			WorldEntity e;
			e.pos = p;
			e.basic_type = MultiViewType::WORLD_OBJECT;
			e.object.filename = fn_rel.no_ext();
			e.object.object = e.object.object = data->session->resource_manager->load_model(e.object.filename);
			data->add_entity(e);
		}
	});

	set_side_panel(new EntityPanel(this));

	/*for (const auto& c: enumerate_classes(session, "ecs.Component"))
		msg_write(c.class_name);
	for (const auto& c: enumerate_classes(session, "ui.Controller"))
		msg_write(c.class_name);*/
}

void ModeWorld::set_side_panel(xhui::Panel* p) {
	if (side_panel) {
	//	session->win->unembed(side_panel);
	}
	side_panel = p;
	session->win->embed("main-grid", 1, 0, side_panel);
}



void ModeWorld::optimize_view() {
	vec3 vmin, vmax;
	data->get_bounding_box(vmin, vmax);
	multi_view->view_port.suggest_for_box(vmin, vmax);
}

#define MODEL_MAX_VERTICES	65536
vec3 tmv[MODEL_MAX_VERTICES*5],pmv[MODEL_MAX_VERTICES*5];
bool tvm[MODEL_MAX_VERTICES*5];


float model_hover_z(Model *o, const vec3& pos, const quaternion& ang, MultiViewWindow* win, const vec2 &mv, vec3 &tp) {
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

float object_hover_distance(const WorldEntity& me, MultiViewWindow* win, const vec2 &mv, vec3 &tp, float &z) {
	Model *o = me.object.object;
	if (!o)
		return -1;
	z = model_hover_z(o, me.pos, me.ang, win, mv, tp);
	return (z < 1) ? 0 : -1;
}

base::optional<Hover> ModeWorld::get_hover(MultiViewWindow* win, const vec2& m) const {
	base::optional<Hover> h;

	float zmin = multi_view->view_port.radius * 2;
	for (const auto& [i, e]: enumerate(data->entities))
		if (e.basic_type == MultiViewType::WORLD_OBJECT) {
			float z;
			vec3 tp;
			float dist = object_hover_distance(e, win, m, tp, z);
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
	s.add({MultiViewType::WORLD_ENTITY, {}});
	for (const auto& [i, e]: enumerate(data->entities)) {
		const auto p = win->project(e.pos);
		if (p.z <= 0 or p.z >= 1)
			continue;
		if (r.inside({p.x, p.y}))
			s[MultiViewType::WORLD_ENTITY].add(i);
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

void draw_mesh(const RenderParams& params, RenderViewData& rvd, const mat4& matrix, VertexBuffer* vb, Material* material, const string& vertex_module = "default") {
	auto shader = rvd.get_shader(material, 0, vertex_module, "");
	auto& rd = rvd.start(params, matrix, shader, *material, 0, PrimitiveTopology::TRIANGLES, vb);
	rd.apply(params);
	params.command_buffer->draw(vb);
}

void ModeWorld::on_prepare_scene(const RenderParams& params) {
	Array<WorldEntity*> data_lights;
	for (auto& e: data->entities)
		if (e.basic_type == MultiViewType::WORLD_LIGHT)
			data_lights.add(&e);

	while (lights.num < data_lights.num) {
		lights.add(new Light(Black, 0, 0));
		lights.back()->owner = new Entity;
	}
	for (const auto& [i, l]: enumerate(data_lights)) {
		lights[i]->owner->pos = l->pos;
		lights[i]->owner->ang = l->ang;
		lights[i]->enabled = l->light.enabled;
		lights[i]->light.col = l->light.col;
		if (!l->light.enabled)
			lights[i]->light.col = Black;
		lights[i]->light.radius = l->light.radius;
		lights[i]->light.theta = l->light.theta;
		if (l->light.type == LightType::DIRECTIONAL)
			lights[i]->light.radius = -1;
		else
			lights[i]->light.col = l->light.col * (l->light.radius * l->light.radius / 100);
		if (l->light.type != LightType::CONE)
			lights[i]->light.theta = -1;
		lights[i]->light.harshness = l->light.harshness;
	}
	multi_view->view_port.scene_view->lights = lights.sub_ref(0, data_lights.num);
}


void ModeWorld::on_draw_win(const RenderParams& params, MultiViewWindow* win) {

	auto& rvd = win->rvd;
	auto cb = params.command_buffer;
	cb->clear(params.area, {data->meta_data.background_color}, 1.0);
	auto dh = win->multi_view->session->drawing_helper;

	for (auto& e: data->entities) {
		if (e.basic_type != MultiViewType::WORLD_TERRAIN)
			continue;
		auto& t = e.terrain;
		t.terrain->prepare_draw(rvd.scene_view->cam->owner->pos);
		auto material = t.terrain->material.get();
		auto vb = t.terrain->vertex_buffer.get();

		auto shader = rvd.get_shader(material, 0, t.terrain->vertex_shader_module, "");
		auto& rd = rvd.start(params, mat4::translation(e.pos), shader, *material, 0, PrimitiveTopology::TRIANGLES, vb);
		cb->push_constant(0, 4, &t.terrain->texture_scale[0].x);
		cb->push_constant(4, 4, &t.terrain->texture_scale[1].x);
		rd.apply(params);
		cb->draw(vb);
	}

	for (auto& e: data->entities)
		if (e.basic_type == MultiViewType::WORLD_OBJECT) {
			auto m = e.object.object;
			for (int k=0; k<m->mesh[0]->sub.num; k++) {
				auto material = m->material[k];
				auto vb = m->mesh[0]->sub[k].vertex_buffer;
				draw_mesh(params, rvd, mat4::translation(e.pos) * mat4::rotation(e.ang), vb, material, m->_template->vertex_shader_module);
			}
		}

	// selection
	for (auto& e: data->entities)
		if (e.basic_type == MultiViewType::WORLD_OBJECT) {
			if (e.is_selected) {
				auto m = e.object.object;
				for (int k=0; k<e.object.object->mesh[0]->sub.num; k++) {
					auto vb = m->mesh[0]->sub[k].vertex_buffer;
					draw_mesh(params, rvd, mat4::translation(e.pos) * mat4::rotation(e.ang), vb, dh->material_selection, m->_template->vertex_shader_module);
				}
			}
		}

	// hover...
	if (multi_view->hover and multi_view->hover->type == MultiViewType::WORLD_ENTITY) {
		auto& e = data->entities[multi_view->hover->index];
		if (e.basic_type == MultiViewType::WORLD_OBJECT) {
			auto m = e.object.object;
			for (int k=0; k<m->mesh[0]->sub.num; k++) {
				auto vb = m->mesh[0]->sub[k].vertex_buffer;
				draw_mesh(params, rvd, mat4::translation(e.pos) * mat4::rotation(e.ang), vb, dh->material_hover, m->_template->vertex_shader_module);
			}
		}
	}


	draw_cameras(win);
	draw_lights(win);

	multi_view->action_controller->draw(params, rvd);
}

void ModeWorld::draw_cameras(MultiViewWindow* win) {
	for (auto& e: data->entities) {
		if (e.basic_type != MultiViewType::WORLD_CAMERA)
			continue;
		auto& c = e.camera;
		//if (c.view_stage < mode->multi_view->view_stage)
		//	continue;

		auto dh = session->drawing_helper;
		auto win = dh->window;

		dh->set_color(color(1, 0.9f, 0.6f, 0.3f));
		dh->set_line_width(3);//scheme.LINE_WIDTH_THIN);
		if (e.is_selected) {
			dh->set_color(Red);
			dh->set_line_width(5);//scheme.LINE_WIDTH_MEDIUM);
		}
		auto q = e.ang;
		float r = win->multi_view->view_port.radius * 0.1f;
		float rr = r * tan(c.fov / 2);
		vec3 ex = q * vec3::EX * rr * 1.333f;
		vec3 ey = q * vec3::EY * rr;
		vec3 ez = q * vec3::EZ * r;

		Array<vec3> points = {
			e.pos, e.pos + ez + ex + ey,
			e.pos, e.pos + ez - ex + ey,
			e.pos, e.pos + ez + ex - ey,
			e.pos, e.pos + ez - ex - ey,
			e.pos + ez + ex + ey, e.pos + ez - ex + ey,
			e.pos + ez - ex + ey, e.pos + ez - ex - ey,
			e.pos + ez - ex - ey, e.pos + ez + ex - ey,
			e.pos + ez + ex - ey, e.pos + ez + ex + ey};
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
	auto dh = win->multi_view->session->drawing_helper;
	dh->draw_lines({p, c + sin(w) * e1 + cos(w) * e2,
		p, c - sin(w) * e1 - cos(w) * e2}, false);
}

const float LIGHT_RADIUS_FACTOR_HI = 0.03f;
const float LIGHT_RADIUS_FACTOR_LO = 0.15f;

void ModeWorld::draw_lights(MultiViewWindow *win) {
	auto dh = session->drawing_helper;
	for (auto &e: data->entities) {
		if (e.basic_type != MultiViewType::WORLD_LIGHT)
			continue;
		auto& l = e.light;
		//if (l.view_stage < multi_view->view_stage)
		//	continue;

		dh->set_color(color(1, 0.9f, 0.6f, 0.3f));
		dh->set_line_width(5);//scheme.LINE_WIDTH_MEDIUM);
		if (e.is_selected) {
			dh->set_color(Red);
			dh->set_line_width(7);//scheme.LINE_WIDTH_THICK);
		}

		if (l.type == LightType::DIRECTIONAL) {
			dh->draw_lines({e.pos, e.pos + e.ang * vec3::EZ * win->multi_view->view_port.radius * 0.1f}, false);
		} else if (l.type == LightType::POINT) {
			//draw_circle(l.pos, win->get_direction(), l.radius);
			dh->draw_circle(e.pos, win->dir(), l.radius * LIGHT_RADIUS_FACTOR_LO);
			dh->draw_circle(e.pos, win->dir(), l.radius * LIGHT_RADIUS_FACTOR_HI);
		} else if (l.type == LightType::CONE) {
			dh->draw_lines({e.pos, e.pos + e.ang * vec3::EZ * l.radius * LIGHT_RADIUS_FACTOR_LO}, false);
			dh->draw_circle(e.pos + e.ang * vec3::EZ * l.radius*LIGHT_RADIUS_FACTOR_LO, e.ang * vec3::EZ, l.radius * tan(l.theta) * LIGHT_RADIUS_FACTOR_LO);
			dh->draw_circle(e.pos + e.ang * vec3::EZ * l.radius*LIGHT_RADIUS_FACTOR_HI, e.ang * vec3::EZ, l.radius * tan(l.theta) * LIGHT_RADIUS_FACTOR_HI);
			draw_tangent_circle(win, e.pos, e.pos + e.ang * vec3::EZ * l.radius*LIGHT_RADIUS_FACTOR_LO, e.ang * vec3::EZ, l.radius * tan(l.theta) * LIGHT_RADIUS_FACTOR_LO);
		}
	}
}


static base::optional<string> world_selection_description(DataWorld* data) {
	int nob = 0, nter = 0, ncam = 0, nent = 0;
	auto sel = data->get_selection();
	if (sel.contains(MultiViewType::WORLD_OBJECT))
		nob = sel[MultiViewType::WORLD_OBJECT].num;
	if (sel.contains(MultiViewType::WORLD_TERRAIN))
		nter = sel[MultiViewType::WORLD_TERRAIN].num;
	if (sel.contains(MultiViewType::WORLD_CAMERA))
		ncam = sel[MultiViewType::WORLD_CAMERA].num;
	if (sel.contains(MultiViewType::WORLD_ENTITY))
		nent = sel[MultiViewType::WORLD_ENTITY].num;
	if (nob + nter + ncam + nent == 0)
		return base::None;
	Array<string> s;
	if (nob > 0)
		s.add(format("%d objects", nob));
	if (nter > 0)
		s.add(format("%d terrains", nter));
	if (ncam > 0)
		s.add(format("%d cameras", ncam));
	if (nent > 0)
		s.add(format("%d entities", nent));
	return implode(s, ", ");
}

void ModeWorld::on_draw_post(Painter* p) {
	p->set_color(Black);
	//p->set_font_size(20);
	//p->draw_str({100, 100}, str(r));

	p->set_color(Blue);
	for (auto& o: data->entities) {
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






