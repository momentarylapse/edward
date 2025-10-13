//
// Created by Michael Ankele on 2025-01-20.
//

#include "ModeWorld.h"
#include "ModeEditTerrain.h"
#include "ModeWorldProperties.h"
#include "dialog/WorldSidePanel.h"
#include "action/ActionWorldMoveSelection.h"
#include <Session.h>
#include <lib/base/iter.h>
#include <lib/kaba/kaba.h>
#include <lib/os/filesystem.h>
#include <view/MultiView.h>
#include <view/DocumentSession.h>
#include "data/DataWorld.h"
#include <lib/yrenderer/Renderer.h>
#include <lib/yrenderer/scene/RenderViewData.h>
#include <lib/yrenderer/scene/SceneView.h>
#include <y/helper/ResourceManager.h>
#include <lib/yrenderer/Material.h>
#include <y/world/Camera.h>
#include <y/world/Light.h>
#include <y/y/Entity.h>
#include <y/world/components/Collider.h>
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
#include <cmath>

#include "lib/mesh/Polygon.h"
#include "lib/mesh/VertexStagingBuffer.h"
#include "lib/mesh/PolygonMesh.h"
#include "lib/mesh/GeometrySphere.h"
#include "lib/mesh/GeometryCylinder.h"
#include "world/components/SolidBody.h"
#include "y/EntityManager.h"


yrenderer::Material* create_material(yrenderer::Context* ctx, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false);

ModeWorld::ModeWorld(DocumentSession* doc) :
	Mode(doc)
{
	auto mvp = new MultiViewPanel(doc);
	multi_view = mvp->multi_view;
	doc->set_document_panel(mvp);

	data = new DataWorld(doc);
	generic_data = data;

	view_mode = ViewMode::Default;

	material_physical = create_material(session->ctx, Black.with_alpha(0.4f), 0.7f, 0.2f, color(1,1,1,0.4f).srgb_to_linear(), true);

	mode_properties = new ModeWorldProperties(this);
}

void ModeWorld::on_set_menu() {
	auto tb = session->win->tool_bar;
	tb->set_by_id("world-toolbar");


	auto menu = xhui::create_resource_menu("menu_world");
	session->win->menu_bar->set_menu(menu);
}


void ModeWorld::on_enter_rec() {
	doc->out_changed >> create_sink([this] {
		on_update_menu();
	});
	auto update_dummies = [this]() {
		// FIXME
		auto list = data->entity_manager->get_component_list<EdwardTag>();
		data->dummy_entities.resize(list.num);
		for (auto&& [i, e]: enumerate(list)) {
			data->dummy_entities[i].pos = e->owner->pos;
			e->entity_index = i;
		}
	};
	auto update_model_refs = [this]() {
		// FIXME
		auto list = data->entity_manager->get_component_list<ModelRef>();
		for (auto mr: list) {
			mr->model = session->resource_manager->load_model(mr->filename);
		}
	};
	data->out_changed >> create_sink(update_dummies);
	data->out_changed >> create_sink(update_model_refs);
	update_dummies();
}

void ModeWorld::on_connect_events_rec() {
	doc->event("mode_world", [this] {
		doc->set_mode(this);
	});
	doc->event("properties", [this] {
		doc->set_mode(mode_properties.get());
		//session->win->open_dialog(new PropertiesDialog(session->win, data));
	});
	doc->event("run-game", [this] {
		Path engine_dir = xhui::config.get_str("EngineDir", "");
		if (engine_dir.is_empty()) {
			session->error("can not run engine. Config 'EngineDir' is not set");
			return;
		}

		auto cmd = format("cd \"%s\"; \"%s\" \"%s\"", session->storage->root_dir, engine_dir | "y", data->filename.basename_no_ext());
		try {
			os::terminal::shell_execute(cmd);
		} catch (Exception &e) {
			session->error(format("failed to run '%s'", cmd));
		}
	});

	/*doc->event("mode_world_terrain", [this] {
		session->set_mode(new ModeEditTerrain(this));
	});*/
}

void ModeWorld::on_leave_rec() {
	doc->out_changed.unsubscribe(this);
}


class WorldOpButtons : public xhui::Panel {
public:
	explicit WorldOpButtons(MultiView* multi_view) : xhui::Panel("world-op-buttons") {
		from_source(R"foodelim(
Dialog world-op-buttons '' propagateevents
	Grid ? '' spacing=20 vertical
		Button mouse-action 'T' image=rf-translate height=50 width=50 padding=7 noexpandx ignorefocus
)foodelim");

		event("mouse-action", [this, multi_view] {
			auto ac = multi_view->action_controller.get();
			const auto mode = ac->action_mode();
			if (mode == MouseActionMode::MOVE) {
				ac->set_action_mode(MouseActionMode::ROTATE);
				set_options("mouse-action", "image=rf-rotate");
			} else if (mode == MouseActionMode::ROTATE) {
				ac->set_action_mode(MouseActionMode::MOVE);
				set_options("mouse-action", "image=rf-translate");
			}
			set_string("mouse-action", multi_view->action_controller->action_name().sub(0, 1).upper());
		});
	}
};

void ModeWorld::on_enter() {
	multi_view->set_allow_select(true);
	multi_view->set_allow_action(true);
	multi_view->set_show_grid(true);
	multi_view->f_hover = [this] (MultiViewWindow* win, const vec2& m) {
		return get_hover(win, m);
	};
	multi_view->f_select = [this] (MultiViewWindow* win, const rect& r) {
		Data::Selection sel;
		sel.set(MultiViewType::WORLD_ENTITY, MultiView::select_points_in_rect(win, r, data->dummy_entities));
		return sel;
	};
	multi_view->f_get_selection_box = [this] (const Data::Selection& sel) -> base::optional<Box> {
		return MultiView::points_get_selection_box(data->dummy_entities, sel[MultiViewType::WORLD_ENTITY]);
	};
	multi_view->f_create_action = [this] {
		return new ActionWorldMoveSelection(data, multi_view->selection);
	};
	multi_view->data_sets = {
		{MultiViewType::WORLD_ENTITY, &data->dummy_entities}//data->entities}
	};
	multi_view->light_mode = MultiView::LightMode::Fixed;

	event_ids.add(doc->document_panel->event_x("area", xhui::event_id::DragDrop, [this] {
		multi_view->hover = multi_view->get_hover(multi_view->hover_window, session->win->drag.m);
		const vec3 p = multi_view->cursor_pos_3d(session->win->drag.m);
		if (session->win->drag.payload.match("add-entity-default-*")) {
			int index = session->win->drag.payload.tail(1)._int();
			data->begin_action_group("new-entity");
			auto e = data->add_entity(p, quaternion::ID);
			if (index == 0) {
				// raw entity
			} else if (index == 1) {
				auto c = data->entity_add_component<Camera>(e);
			} else if (index == 2) {
				auto l = data->entity_add_component<Light>(e, {{{"radius", 0.0f}, {"theta", 0.0f}}});
				l->light.init(yrenderer::LightType::DIRECTIONAL, White);
			} else if (index == 3) {
				auto l = data->entity_add_component<Light>(e, {{{"radius", multi_view->view_port.radius * 1.3f}, {"theta", 0.0f}}});
				float r = multi_view->view_port.radius * 1.3f;
				l->light.init(yrenderer::LightType::POINT, White * (r*r/100.0f));
			} else if (index == 4) {
				auto l = data->entity_add_component<Light>(e);
				float r = multi_view->view_port.radius * 1.3f;
				l->light.init(yrenderer::LightType::CONE, White * (r*r/100.0f), 0.5f);
			} else if (index == 5) {
				auto t = data->entity_add_component<TerrainRef>(e);
	//			e.basic_type = MultiViewType::WORLD_TERRAIN;
	//			e.terrain.terrain = new Terrain(16, 16, {10, 0, 10}, session->resource_manager->load_material(""));
			}
			data->end_action_group();
		} else if (session->win->drag.payload.match("filename:*.model")) {
			Path filename = session->win->drag.payload.sub_ref(9);
			auto fn_rel = filename.relative_to(session->storage->get_root_dir(FD_MODEL));
			session->set_message(str(fn_rel));
			auto e = data->add_entity(p, quaternion::ID);
			auto c = data->entity_add_component<ModelRef>(e);
			c->filename = fn_rel;
			c->model = session->resource_manager->load_model(c->filename);

			// suggest automatic components...
			// well... SolidBody will be removed soon!
			if (c->model)
				data->_entity_apply_components(e, c->model->_template->components);
		} else if (session->win->drag.payload.match("filename:*.map")) {
			Path filename = session->win->drag.payload.sub_ref(9);
			auto fn_rel = filename.relative_to(session->storage->get_root_dir(FD_TERRAIN));
			session->set_message(str(fn_rel));
			auto e = data->add_entity(p, quaternion::ID);
			auto c = data->entity_add_component<TerrainRef>(e);
			c->filename = fn_rel;
			c->terrain = new Terrain(session->ctx, c->filename.no_ext());

			// suggest automatic components
			data->_entity_apply_components(e, LevelData::auto_terrain_components());
		} else if (session->win->drag.payload.match("filename:*.template")) {
			Path filename = session->win->drag.payload.sub_ref(9);
			auto fn_rel = filename.relative_to(session->storage->get_root_dir(FD_MODEL));
			session->set_message(str(fn_rel));
			auto e = data->add_entity(p, quaternion::ID);
			auto t = LevelData::load_template(filename);
			data->_entity_apply_components(e, t.components);

			if (auto m = e->get_component<ModelRef>()) {
				if (m->filename)
					m->model = session->resource_manager->load_model(m->filename.no_ext());
			}
		}
	}));

	set_side_panel(new WorldSidePanel(this));
	set_overlay_panel(new WorldOpButtons(multi_view));
}


void ModeWorld::on_leave() {
	set_side_panel(nullptr);
	set_overlay_panel(nullptr);

	data->out_changed.unsubscribe(this);

	for (int uid: event_ids) {
		doc->document_panel->remove_event_handler(uid);
	}
	event_ids.clear();
}


void ModeWorld::on_update_menu() {
	auto win = session->win;

	win->check("mode_world", doc->cur_mode == this);
	win->check("properties", doc->cur_mode == mode_properties.get());
}


void ModeWorld::optimize_view() {
	multi_view->view_port.suggest_for_box(data->get_bounding_box());
	multi_view->view_port.ang = quaternion::rotation(vec3(0.7f,0,0));
}

void ModeWorld::set_view_mode(ViewMode mode) {
	view_mode = mode;
	out_changed();
	session->win->request_redraw();
}


#define MODEL_MAX_VERTICES	65536
vec3 tmv[MODEL_MAX_VERTICES*5],pmv[MODEL_MAX_VERTICES*5];
bool tvm[MODEL_MAX_VERTICES*5];


float model_hover_z(const Model *o, const mat4& matrix, MultiViewWindow* win, const vec2 &mv, vec3 &tp) {
	if (!o)
		return -1;
	int d = 0;//o->_detail_;
	if ((d<0) or (d>2))
		return -1;
	for (int i=0;i<o->mesh[d]->vertex.num;i++) {
		tmv[i] = matrix * o->mesh[d]->vertex[i];
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

/*float object_hover_distance(const WorldEntity& me, MultiViewWindow* win, const vec2 &mv, vec3 &tp, float &z) {
	Model *o = me.object.object;
	if (!o)
		return -1;
	z = model_hover_z(o, me.pos, me.ang, win, mv, tp);
	return (z < 1) ? 0 : -1;
}*/

base::optional<Hover> ModeWorld::get_hover(MultiViewWindow* win, const vec2& mouse) const {
	base::optional<Hover> h;

	float zmin = 1;//multi_view->view_port.radius * 2;


	const auto models = data->entity_manager->get_component_list<ModelRef>();
	for (auto mr: models)
		if (auto m = mr->model) {
			vec3 tp;
			float z = model_hover_z(m, mr->owner->get_matrix(), win, mouse, tp);
			//float dist = object_hover_distance(e, win, m, tp, z);
			if (z >= 0 and z < zmin) {
				zmin = z;
				int i = mr->owner->get_component<EdwardTag>()->entity_index;
				h = {MultiViewType::WORLD_ENTITY, i, tp};
			}
		}
	return h;
}

Data::Selection ModeWorld::get_selection(MultiViewWindow* win, const rect& _r) const {
	auto r = _r.canonical();
	Data::Selection s;
	s.add({MultiViewType::WORLD_ENTITY, {}});
	for (const auto& [i, e]: enumerate(data->entity_manager->entities)) {
		const auto p = win->project(e->pos);
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
	auto data_lights = data->entity_manager->get_component_list<Light>();

	while (lights.num < data_lights.num) {
		lights.add(new yrenderer::Light);
	}
	for (const auto& [i, l]: enumerate(data_lights)) {
		*lights[i] = l->light;
		lights[i]->pos = l->owner->pos;
		lights[i]->_ang = l->owner->ang;
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

	const auto terrains = data->entity_manager->get_component_list<TerrainRef>();
	for (auto tr: terrains)
		if (auto t = tr->terrain) {
			t->prepare_draw(rvd.scene_view->main_camera_params.pos);
			auto material = t->material.get();
			auto vb = t->vertex_buffer.get();

			auto shader = rvd.get_shader(material, 0, t->vertex_shader_module, "");
			auto& rd = rvd.start(params, tr->owner->get_matrix(), shader, *material, 0, ygfx::PrimitiveTopology::TRIANGLES, vb);
#ifdef USING_VULKAN
			cb->push_constant(0, 12, &t->texture_scale[0].x);
			cb->push_constant(16, 12, &t->texture_scale[1].x);
#else
			shader->set_floats("pattern0", &t->texture_scale[0].x, 3);
			shader->set_floats("pattern1", &t->texture_scale[1].x, 3);
#endif
			rd.draw_triangles(params, vb);
		}

	const auto models = data->entity_manager->get_component_list<ModelRef>();
	for (auto mr: models)
		if (auto m = mr->model)
			for (int k=0; k<m->mesh[0]->sub.num; k++) {
				auto material = m->material[k];
				auto vb = m->mesh[0]->sub[k].vertex_buffer;
				dh->draw_mesh(params, rvd, mr->owner->get_matrix(), vb, material, 0, "default");
			}

	if (view_mode == ViewMode::Physical) {
		for (auto mr: models)
			if (auto m = mr->model) {
				if (auto p = m->_template->physical_mesh.get()) {
					if (!physical_vertex_buffers.contains(p)) {
						auto _vb = new ygfx::VertexBuffer("3f,3f,2f");
						PolygonMesh mesh;
						for (const auto& v: p->vertex)
							mesh.vertices.add(MeshVertex(v));
						for (const auto& poly: p->poly) {
							Polygon polygon;
							for (int f=0; f<poly.num_faces; f++) {
								polygon.side.resize(poly.face[f].num_vertices);
								for (int k=0; k<poly.face[f].num_vertices; k++)
									polygon.side[k].vertex = poly.face[f].index[k];
								mesh.polygons.add(polygon);
							}
						}
						for (const auto& b: p->balls)
							mesh.add(GeometrySphere(p->vertex[b.index], b.radius, 8));
						for (const auto& c: p->cylinders)
							mesh.add(GeometryCylinder(p->vertex[c.index[0]], p->vertex[c.index[1]], c.radius, 1, 32, c.round ? GeometryCylinder::END_ROUND : GeometryCylinder::END_FLAT));

						mesh.build(_vb);
						physical_vertex_buffers.set(p, _vb);
					}
					auto vb = physical_vertex_buffers[p];
					dh->draw_mesh(params, rvd, mr->owner->get_matrix(), vb, material_physical, 0, "default");
				}
			}
				/*for (int k=0; k<m->mesh[0]->sub.num; k++) {
					auto material = m->material[k];
					auto vb = m->mesh[0]->sub[k].vertex_buffer;
					dh->draw_mesh(params, rvd, mr->owner->get_matrix(), vb, material, 0, "default");
				}*/
	}

	// selection
	for (auto mr: models) {
		int i = mr->owner->get_component<EdwardTag>()->entity_index;
		if (sel.contains(i)) {
			if (auto m = mr->model)
				for (int k=0; k<m->mesh[0]->sub.num; k++) {
					auto vb = m->mesh[0]->sub[k].vertex_buffer;
					dh->draw_mesh(params, rvd, mr->owner->get_matrix(), vb, dh->material_selection, 0, "default");
				}
		}
	}

	// hover...
	if (multi_view->hover and multi_view->hover->type == MultiViewType::WORLD_ENTITY) {
		auto e = data->entity_manager->entities[multi_view->hover->index];
		if (auto mr = e->get_component<ModelRef>()) {
			if (auto m = mr->model) {
				for (int k=0; k<m->mesh[0]->sub.num; k++) {
					auto vb = m->mesh[0]->sub[k].vertex_buffer;
					dh->draw_mesh(params, rvd, e->get_matrix(), vb, dh->material_hover, 0, "default");
				}
			}
		}
	}

	draw_cameras(win);
	draw_lights(win);
}

void ModeWorld::on_draw_shadow(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) {
	auto dh = multi_view->session->drawing_helper;

	const auto terrains = data->entity_manager->get_component_list<TerrainRef>();
	for (auto tr: terrains)
		if (auto t = tr->terrain) {
			t->prepare_draw(rvd.scene_view->main_camera_params.pos);
			auto vb = t->vertex_buffer.get();
			dh->draw_mesh(params, rvd, tr->owner->get_matrix(), vb, dh->material_shadow);
		}

	const auto models = data->entity_manager->get_component_list<ModelRef>();
	for (auto mr: models)
		if (auto m = mr->model)
			for (int k=0; k<m->mesh[0]->sub.num; k++) {
				auto vb = m->mesh[0]->sub[k].vertex_buffer;
				dh->draw_mesh(params, rvd, mr->owner->get_matrix(), vb, dh->material_shadow, 0, "default");
			}
}


void ModeWorld::draw_cameras(MultiViewWindow* win) {
	auto dh = session->drawing_helper;
	const auto& sel = multi_view->selection[MultiViewType::WORLD_ENTITY];
	for (const auto c: data->entity_manager->get_component_list<Camera>()) {
		//if (c.view_stage < mode->multi_view->view_stage)
		//	continue;
		int i = c->owner->get_component<EdwardTag>()->entity_index;

		dh->set_color(DrawingHelper::COLOR_X);
		dh->set_line_width(DrawingHelper::LINE_MEDIUM);
		if (sel.contains(i)) {
			dh->set_color(Red);
			dh->set_line_width(DrawingHelper::LINE_THICK);
		}
		auto q = c->owner->ang;
		float r = win->multi_view->view_port.radius * 0.1f;
		float rr = r * tanf(c->fov / 2);
		vec3 ex = q * vec3::EX * rr * 1.333f;
		vec3 ey = q * vec3::EY * rr;
		vec3 ez = q * vec3::EZ * r;

		const vec3 pos = c->owner->pos;
		Array<vec3> points = {
			pos, pos + ez + ex + ey,
			pos, pos + ez - ex + ey,
			pos, pos + ez + ex - ey,
			pos, pos + ez - ex - ey,
			pos + ez + ex + ey, pos + ez - ex + ey,
			pos + ez - ex + ey, pos + ez - ex - ey,
			pos + ez - ex - ey, pos + ez + ex - ey,
			pos + ez + ex - ey, pos + ez + ex + ey};
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
		float w = (float)i * 2 * pi / (float)N;
		vec2 pp = win->project(c + sinf(w) * e1 + cosf(w) * e2).xy();
		if ((pp - pc).length() > d_max) {
			i_max = i;
			d_max = (pp - pc).length();
		}
	}
	float w = (float)i_max * 2 * pi / (float)N;
	auto dh = win->multi_view->session->drawing_helper;
	dh->draw_lines({p, c + sinf(w) * e1 + cosf(w) * e2,
		p, c - sinf(w) * e1 - cosf(w) * e2}, false);
}

const float LIGHT_RADIUS_FACTOR_HI = 0.03f;
const float LIGHT_RADIUS_FACTOR_LO = 0.15f;

void ModeWorld::draw_lights(MultiViewWindow *win) {
	auto dh = session->drawing_helper;
	const auto& sel = multi_view->selection[MultiViewType::WORLD_ENTITY];
	for (const auto l: data->entity_manager->get_component_list<Light>()) {
		//if (l.view_stage < multi_view->view_stage)
		//	continue;
		int i = l->owner->get_component<EdwardTag>()->entity_index;

		dh->set_color(DrawingHelper::COLOR_X);
		dh->set_line_width(DrawingHelper::LINE_THICK);
		if (sel.contains(i)) {
			dh->set_color(Red);
			dh->set_line_width(DrawingHelper::LINE_EXTRA_THICK);
		}

		const vec3 pos = l->owner->pos;
		const quaternion ang = l->owner->ang;
		const float radius = l->light.radius();
		if (l->light.type == yrenderer::LightType::DIRECTIONAL) {
			dh->draw_lines({pos, pos + ang * vec3::EZ * win->multi_view->view_port.radius * 0.1f}, false);
		} else if (l->light.type == yrenderer::LightType::POINT) {
			//draw_circle(l.pos, win->get_direction(), l.radius);
			dh->draw_circle(pos, win->direction(), radius * LIGHT_RADIUS_FACTOR_LO);
			dh->draw_circle(pos, win->direction(), radius * LIGHT_RADIUS_FACTOR_HI);
		} else if (l->light.type == yrenderer::LightType::CONE) {
			const float theta = l->light.theta;
			dh->draw_lines({pos, pos + ang * vec3::EZ * radius * LIGHT_RADIUS_FACTOR_LO}, false);
			dh->draw_circle(pos + ang * vec3::EZ * radius*LIGHT_RADIUS_FACTOR_LO, ang * vec3::EZ, radius * tanf(theta) * LIGHT_RADIUS_FACTOR_LO);
			dh->draw_circle(pos + ang * vec3::EZ * radius*LIGHT_RADIUS_FACTOR_HI, ang * vec3::EZ, radius * tanf(theta) * LIGHT_RADIUS_FACTOR_HI);
			draw_tangent_circle(win, pos, pos + ang * vec3::EZ * radius*LIGHT_RADIUS_FACTOR_LO, ang * vec3::EZ, radius * tanf(theta) * LIGHT_RADIUS_FACTOR_LO);
		}
	}
}


static base::optional<string> world_selection_description(DataWorld* data, const Data::Selection& sel) {
	int nob = 0, nter = 0, ncam = 0, nent = 0;
	if (sel.contains(MultiViewType::WORLD_OBJECT))
		nob = sel[MultiViewType::WORLD_OBJECT].num;
	if (sel.contains(MultiViewType::WORLD_TERRAIN))
		nter = sel[MultiViewType::WORLD_TERRAIN].num;
//	if (sel.contains(MultiViewType::WORLD_CAMERA))
//		ncam = sel[MultiViewType::WORLD_CAMERA].num;
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
		data->dummy_entities,
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
		if (temp.entities.num == 0)
			session->set_message("nothing selected");
		else
			session->set_message("copied: " + world_selection_description(data, multi_view->selection).value_or("???"));
	}
	if (id == "paste") {
		if (temp.entities.num == 0) {
			session->set_message("nothing to paste");
		} else {
			data->paste(temp, &multi_view->selection);
			multi_view->update_selection_box();
			session->set_message("pasted: " + world_selection_description(data, multi_view->selection).value_or("???"));
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
	if (key == xhui::KEY_P) {
		if (view_mode == ViewMode::Physical)
			set_view_mode(ViewMode::Default);
		else
			set_view_mode(ViewMode::Physical);
	}
}






