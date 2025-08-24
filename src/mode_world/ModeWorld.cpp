//
// Created by Michael Ankele on 2025-01-20.
//

#include "ModeWorld.h"
#include "ModeEditTerrain.h"
#include "ModeScripting.h"
#include "dialog/EntityPanel.h"
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
#include <lib/yrenderer/Renderer.h>
#include <lib/yrenderer/scene/RenderViewData.h>
#include <lib/yrenderer/scene/SceneView.h>
#include <y/helper/ResourceManager.h>
#include <lib/yrenderer/Material.h>
#include <y/world/Camera.h>
#include <y/world/Light.h>
#include <y/y/Entity.h>
#include <y/world/Model.h>
#include <y/world/ModelManager.h>
#include <y/world/Terrain.h>
#include <y/world/World.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/os/msg.h>
#include <lib/os/terminal.h>
#include <lib/xhui/config.h>
#include <lib/xhui/Resource.h>
#include <lib/xhui/controls/MenuBar.h>
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

	mode_scripting = new ModeScripting(this);
}

Array<WorldScriptVariable> load_variables(const kaba::Class* c) {
	Array<WorldScriptVariable> variables;
	for (auto cc: weak(c->constants))
		if (cc->type.get() == kaba::TypeString and cc->name == "PARAMETERS") {
			auto params = cc->as_string().explode(",");
			for (auto& v: c->elements)
				if (sa_contains(params, v.name)) {
					if (v.type == kaba::TypeString or v.type == kaba::TypeFloat32 or v.type == kaba::TypeInt32)
						variables.add({v.name, v.type->name});
				}
		}
	return variables;
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
				if (c->is_derived_from_s(full_base_class) and c->name != base_class) {
					auto variables = load_variables(c);
					r.add({f, c->name, variables});
				}
			}
		} catch (Exception &e) {
			msg_error(e.message());
		}
	}
	return r;
}


void update_class(Session* session, ScriptInstanceData& _c) {
	try {
		auto context = ownify(kaba::Context::create());
		auto s = context->load_module(session->storage->root_dir_kind[FD_SCRIPT] | _c.filename, true);
		for (auto c: s->classes())
			if (c->name == _c.class_name) {
				auto variables = load_variables(c);
				for (const auto& v: variables) {
					bool has = false;
					for (const auto& x: _c.variables)
						if (x.name == v.name)
							has = true;
					if (!has)
						_c.variables.add(v);
				}
			}
	} catch (Exception &e) {
		msg_error(e.message());
	}
}

void ModeWorld::on_enter_rec() {
	session->out_changed >> create_sink([this] {
		update_menu();
	});

	event_ids_rec.add(session->win->event("mode_world", [this] {
		session->set_mode(this);
	}));
	event_ids_rec.add(session->win->event("mode-world-scripting", [this] {
		session->set_mode(mode_scripting.get());
	}));
	session->win->event("properties", [this] {
		session->win->open_dialog(new PropertiesDialog(session->win, data));
	});
	session->win->event("run-game", [this] {
		Path engine_dir = xhui::config.get_str("EngineDir", "");
		if (engine_dir.is_empty()) {
			session->error("cn not run engine. Config 'EngineDir' is not set");
			return;
		}

		auto cmd = format("cd \"%s\"; \"%s\" \"%s\"", session->storage->root_dir, engine_dir | "y", data->filename.basename_no_ext());
		try {
			os::terminal::shell_execute(cmd);
		} catch (Exception &e) {
			session->error(format("failed to run '%s'", cmd));
		}
	});

	/*session->win->event("mode_world_terrain", [this] {
		session->set_mode(new ModeEditTerrain(this));
	});*/
}

void ModeWorld::on_leave_rec() {
	session->out_changed.unsubscribe(this);

	for (int uid: event_ids_rec)
		session->win->remove_event_handler(uid);
	event_ids_rec.clear();
}


void ModeWorld::on_enter() {
	multi_view->set_allow_select(true);
	multi_view->set_allow_action(true);
	multi_view->set_show_grid(true);
	multi_view->f_hover = [this] (MultiViewWindow* win, const vec2& m) {
		return get_hover(win, m);
	};
	multi_view->f_select = [this] (MultiViewWindow* win, const rect& r) {
		Data::Selection sel;
		sel.set(MultiViewType::WORLD_ENTITY, MultiView::select_points_in_rect(win, r, data->entities));
		return sel;
	};
	multi_view->f_get_selection_box = [this] (const Data::Selection& sel) -> base::optional<Box> {
		return MultiView::points_get_selection_box(data->entities, sel[MultiViewType::WORLD_ENTITY]);
	};
	multi_view->f_create_action = [this] {
		return new ActionWorldMoveSelection(data, multi_view->selection);
	};
	multi_view->data_sets = {
		{MultiViewType::WORLD_ENTITY, &data->entities}
	};
	multi_view->light_mode = MultiView::LightMode::Fixed;

	auto tb = session->win->toolbar;
	tb->set_by_id("world-toolbar");


	auto menu_bar = (xhui::MenuBar*)session->win->get_control("menu");
	auto menu = xhui::create_resource_menu("menu_world");
	menu_bar->set_menu(menu);

	event_ids.add(session->win->event_x("area", xhui::event_id::DragDrop, [this] {
		multi_view->hover = multi_view->get_hover(multi_view->hover_window, session->win->drag.m);
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
			} else if (index <= 4) {
				e.basic_type = MultiViewType::WORLD_LIGHT;
				e.light.col = White;
				e.light.type = yrenderer::LightType::DIRECTIONAL;
				e.light.radius = 0;
				e.light.theta = 0;
				if (index == 3) {
					e.light.type = yrenderer::LightType::POINT;
					e.light.radius = multi_view->view_port.radius * 1.3f;
				} else if (index == 4) {
					e.light.type = yrenderer::LightType::CONE;
					e.light.radius = multi_view->view_port.radius * 0.3f;
					e.light.theta = 0.5f;
				}
				e.light.harshness = 1;
				e.light.enabled = true;
			} else if (index == 5) {
				e.basic_type = MultiViewType::WORLD_TERRAIN;
				e.terrain.terrain = new Terrain(16, 16, {10, 0, 10}, session->resource_manager->load_material(""));
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
			e.object.object = e.object.object = session->resource_manager->load_model(e.object.filename);
			data->add_entity(e);
		}
	}));

	set_side_panel(new EntityPanel(this));
}


void ModeWorld::on_leave() {
	//session->win->unembed(dialog);
	set_side_panel(nullptr);

	data->out_changed.unsubscribe(this);

	for (int uid: event_ids)
		session->win->remove_event_handler(uid);
}


void ModeWorld::update_menu() {
	auto win = session->win;

	win->check("mode_world", session->cur_mode == this);
	win->check("mode-world-scripting", session->cur_mode == mode_scripting.get());
}


void ModeWorld::optimize_view() {
	multi_view->view_port.suggest_for_box(data->get_bounding_box());
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
				h = {MultiViewType::WORLD_ENTITY, i, tp};
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

void draw_mesh(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, const mat4& matrix, ygfx::VertexBuffer* vb, yrenderer::Material* material, const string& vertex_module = "default") {
	auto shader = rvd.get_shader(material, 0, vertex_module, "");
	auto& rd = rvd.start(params, matrix, shader, *material, 0, ygfx::PrimitiveTopology::TRIANGLES, vb);
	rd.draw_triangles(params, vb);
}

void ModeWorld::on_prepare_scene(const yrenderer::RenderParams& params) {
	Array<WorldEntity*> data_lights;
	for (auto& e: data->entities)
		if (e.basic_type == MultiViewType::WORLD_LIGHT)
			data_lights.add(&e);

	while (lights.num < data_lights.num) {
		lights.add(new yrenderer::Light);
	}
	for (const auto& [i, l]: enumerate(data_lights)) {
		lights[i]->light.pos = l->pos;
		lights[i]->_ang = l->ang;
		lights[i]->enabled = l->light.enabled;
		lights[i]->allow_shadow = (l->light.type == yrenderer::LightType::DIRECTIONAL);
		lights[i]->light.col = l->light.col;
		if (!l->light.enabled)
			lights[i]->light.col = Black;
		lights[i]->light.radius = l->light.radius;
		lights[i]->light.theta = l->light.theta;
		if (l->light.type == yrenderer::LightType::DIRECTIONAL)
			lights[i]->light.radius = -1;
		else
			lights[i]->light.col = l->light.col * (l->light.radius * l->light.radius / 100);
		if (l->light.type != yrenderer::LightType::CONE)
			lights[i]->light.theta = -1;
		lights[i]->light.harshness = l->light.harshness;
	}
	multi_view->lights = lights.sub_ref(0, data_lights.num);
}

void ModeWorld::on_draw_background(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) {
	rvd.clear(params, {data->meta_data.background_color});
}

void ModeWorld::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) {

	auto& rvd = win->rvd();
	auto dh = multi_view->session->drawing_helper;
	const auto& sel = multi_view->selection[MultiViewType::WORLD_ENTITY];

#ifdef USING_VULKAN
	auto cb = params.command_buffer;
#endif

	for (auto& e: data->entities) {
		if (e.basic_type != MultiViewType::WORLD_TERRAIN)
			continue;
		auto& t = e.terrain;
		t.terrain->prepare_draw(rvd.scene_view->main_camera_params.pos);
		auto material = t.terrain->material.get();
		auto vb = t.terrain->vertex_buffer.get();

		auto shader = rvd.get_shader(material, 0, t.terrain->vertex_shader_module, "");
		auto& rd = rvd.start(params, mat4::translation(e.pos), shader, *material, 0, ygfx::PrimitiveTopology::TRIANGLES, vb);
#ifdef USING_VULKAN
		cb->push_constant(0, 12, &t.terrain->texture_scale[0].x);
		cb->push_constant(16, 12, &t.terrain->texture_scale[1].x);
#else
		shader->set_floats("pattern0", &t.terrain->texture_scale[0].x, 3);
		shader->set_floats("pattern1", &t.terrain->texture_scale[1].x, 3);
#endif
		rd.draw_triangles(params, vb);
	}

	for (auto& e: data->entities)
		if (e.basic_type == MultiViewType::WORLD_OBJECT) {
			auto m = e.object.object;
			for (int k=0; k<m->mesh[0]->sub.num; k++) {
				auto material = m->material[k];
				auto vb = m->mesh[0]->sub[k].vertex_buffer;
				dh->draw_mesh(params, rvd, mat4::translation(e.pos) * mat4::rotation(e.ang), vb, material, 0, m->_template->vertex_shader_module);
			}
		}

	// selection
	for (const auto& [i, e]: enumerate(data->entities))
		if (e.basic_type == MultiViewType::WORLD_OBJECT) {
			if (sel.contains(i)) {
				auto m = e.object.object;
				for (int k=0; k<e.object.object->mesh[0]->sub.num; k++) {
					auto vb = m->mesh[0]->sub[k].vertex_buffer;
					dh->draw_mesh(params, rvd, mat4::translation(e.pos) * mat4::rotation(e.ang), vb, dh->material_selection, 0, m->_template->vertex_shader_module);
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
				dh->draw_mesh(params, rvd, mat4::translation(e.pos) * mat4::rotation(e.ang), vb, dh->material_hover, 0, m->_template->vertex_shader_module);
			}
		}
	}

	draw_cameras(win);
	draw_lights(win);
}

void ModeWorld::on_draw_shadow(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) {
	auto dh = multi_view->session->drawing_helper;

	for (auto& e: data->entities) {
		if (e.basic_type != MultiViewType::WORLD_TERRAIN)
			continue;
		auto& t = e.terrain;
		t.terrain->prepare_draw(rvd.scene_view->main_camera_params.pos);
		auto vb = t.terrain->vertex_buffer.get();
		dh->draw_mesh(params, rvd, mat4::translation(e.pos), vb, dh->material_shadow);
	}

	for (auto& e: data->entities)
		if (e.basic_type == MultiViewType::WORLD_OBJECT) {
			auto m = e.object.object;
			for (int k=0; k<m->mesh[0]->sub.num; k++) {
				auto vb = m->mesh[0]->sub[k].vertex_buffer;
				dh->draw_mesh(params, rvd, mat4::translation(e.pos) * mat4::rotation(e.ang), vb, dh->material_shadow, 0, m->_template->vertex_shader_module);
			}
		}
}


void ModeWorld::draw_cameras(MultiViewWindow* win) {
	const auto& sel = multi_view->selection[MultiViewType::WORLD_ENTITY];
	for (const auto& [i,e]: enumerate(data->entities)) {
		if (e.basic_type != MultiViewType::WORLD_CAMERA)
			continue;
		auto& c = e.camera;
		//if (c.view_stage < mode->multi_view->view_stage)
		//	continue;

		auto dh = session->drawing_helper;
		auto win = dh->window;

		dh->set_color(DrawingHelper::COLOR_X);
		dh->set_line_width(DrawingHelper::LINE_MEDIUM);
		if (sel.contains(i)) {
			dh->set_color(Red);
			dh->set_line_width(DrawingHelper::LINE_THICK);
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
	const auto& sel = multi_view->selection[MultiViewType::WORLD_ENTITY];
	for (const auto& [i,e]: enumerate(data->entities)) {
		if (e.basic_type != MultiViewType::WORLD_LIGHT)
			continue;
		auto& l = e.light;
		//if (l.view_stage < multi_view->view_stage)
		//	continue;

		dh->set_color(DrawingHelper::COLOR_X);
		dh->set_line_width(DrawingHelper::LINE_THICK);
		if (sel.contains(i)) {
			dh->set_color(Red);
			dh->set_line_width(DrawingHelper::LINE_EXTRA_THICK);
		}

		if (l.type == yrenderer::LightType::DIRECTIONAL) {
			dh->draw_lines({e.pos, e.pos + e.ang * vec3::EZ * win->multi_view->view_port.radius * 0.1f}, false);
		} else if (l.type == yrenderer::LightType::POINT) {
			//draw_circle(l.pos, win->get_direction(), l.radius);
			dh->draw_circle(e.pos, win->direction(), l.radius * LIGHT_RADIUS_FACTOR_LO);
			dh->draw_circle(e.pos, win->direction(), l.radius * LIGHT_RADIUS_FACTOR_HI);
		} else if (l.type == yrenderer::LightType::CONE) {
			dh->draw_lines({e.pos, e.pos + e.ang * vec3::EZ * l.radius * LIGHT_RADIUS_FACTOR_LO}, false);
			dh->draw_circle(e.pos + e.ang * vec3::EZ * l.radius*LIGHT_RADIUS_FACTOR_LO, e.ang * vec3::EZ, l.radius * tan(l.theta) * LIGHT_RADIUS_FACTOR_LO);
			dh->draw_circle(e.pos + e.ang * vec3::EZ * l.radius*LIGHT_RADIUS_FACTOR_HI, e.ang * vec3::EZ, l.radius * tan(l.theta) * LIGHT_RADIUS_FACTOR_HI);
			draw_tangent_circle(win, e.pos, e.pos + e.ang * vec3::EZ * l.radius*LIGHT_RADIUS_FACTOR_LO, e.ang * vec3::EZ, l.radius * tan(l.theta) * LIGHT_RADIUS_FACTOR_LO);
		}
	}
}


static base::optional<string> world_selection_description(DataWorld* data, const Data::Selection& sel) {
	int nob = 0, nter = 0, ncam = 0, nent = 0;
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
	drawing2d::draw_data_points(p, multi_view->active_window,
		data->entities,
		MultiViewType::WORLD_ENTITY,
		multi_view->hover,
		multi_view->selection[MultiViewType::WORLD_ENTITY]);

	if (auto s = world_selection_description(data, multi_view->selection))
		draw_info(p, "selected: " + *s);
}

void ModeWorld::on_command(const string& id) {
	if (id == "new")
		session->universal_new(FD_WORLD);
	if (id == "open")
		session->universal_open(FD_WORLD);
	if (id == "save")
		session->storage->save(data->filename, data);
	if (id == "save-as")
		session->storage->save_as(data);
	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();
	if (id == "copy") {
		data->copy(temp, multi_view->selection);
		if (temp.is_empty())
			session->set_message("nothing selected");
		else
			session->set_message("copied: " + *world_selection_description(data, multi_view->selection));
	}
	if (id == "paste") {
		multi_view->clear_selection();
		if (temp.is_empty()) {
			session->set_message("nothing to paste");
		} else {
			data->paste(temp);
			session->set_message("pasted: " + *world_selection_description(data, multi_view->selection));
		}
	}
	if (id == "delete") {
		if (auto s = world_selection_description(data, multi_view->selection)) {
			data->delete_selection(multi_view->selection);
			multi_view->clear_selection();
			session->set_message("deleted: " + *s);
		} else {
			session->set_message("nothing selected");
		}
	}
}

void ModeWorld::on_key_down(int key) {
}






