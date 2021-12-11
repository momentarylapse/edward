/*
 * ModeWorld.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "ModeWorld.h"
#include "../../Edward.h"
#include "../../Storage/Storage.h"
#include "../../MultiView/MultiView.h"
#include "../../MultiView/Window.h"
#include "../../MultiView/DrawingHelper.h"
#include "../../MultiView/ColorScheme.h"
#include "../../Data/World/DataWorld.h"
#include "../../Data/World/DataCamera.h"
#include "../../Data/World/WorldLink.h"
#include "../../Data/World/WorldLight.h"
#include "../../Data/World/WorldObject.h"
#include "../../Data/World/WorldTerrain.h"
#include "../../Data/World/WorldCamera.h"
#include "../../lib/nix/nix.h"
#include "../../lib/math/vec2.h"
#include "../../y/Camera.h"
#include "../../y/World.h"
#include "../../y/Material.h"
#include "../../y/Model.h"
#include "../../y/Object.h"
#include "../../y/Terrain.h"
#include "Dialog/TerrainPropertiesDialog.h"
#include "Dialog/TerrainHeightmapDialog.h"
#include "Dialog/LightmapDialog.h"
#include "Dialog/WorldPropertiesDialog.h"
#include "Dialog/WorldObjectListPanel.h"
#include "Creation/ModeWorldCreateObject.h"
#include "Creation/ModeWorldCreateTerrain.h"
#include "Creation/ModeWorldCreateLink.h"
#include "Creation/ModeWorldCreateLight.h"
#include "Camera/ModeWorldCamera.h"
#include "../../Action/World/ActionWorldEditData.h"
#include "../../Action/World/ActionWorldSetEgo.h"
#include "Terrain/ModeWorldTerrain.h"
#include "../../lib/kaba/kaba.h"

ModeWorld *mode_world = NULL;


#define RotationMouseSpeed			0.002f
#define TerrainHeightMapFactorDec	4
#define TerrainTextureScaleDec		5
#define TerrainSizeDec				3
#define TerrainPatternDec			4
#define OSelectionAlpha				0.25f
#define OMouseOverAlpha				0.25f
#define TSelectionAlpha				0.20f
#define TMouseOverAlpha				0.20f


namespace nix {
extern Shader *current_shader;
}



ModeWorld::ModeWorld(MultiView::MultiView *mv) :
	Mode<DataWorld>("World", NULL, new DataWorld, mv, "menu_world") {
	data->subscribe(this, [=]{ data->update_data(); });

	world_dialog = nullptr;
	dialog = nullptr;
	mouse_action = -1;

	show_effects = true;
	TerrainShowTextureLevel = -1;

	mode_world_camera = new ModeWorldCamera(this, new DataCamera);
	mode_world_terrain = new ModeWorldTerrain(this);
}

ModeWorld::~ModeWorld() {
	data->unsubscribe(this);
}

bool ModeWorld::save_as() {
	for (auto &t: data->terrains)
		if (t.filename == "") {
			if (!storage->file_dialog(FD_TERRAIN, true, true))
				return false;
			if (!t.save(storage->dialog_file_complete))
				return false;
		}
	return storage->save_as(data);
}



void ModeWorld::on_command(const string & id) {
	if (id == "new")
		_new();
	if (id == "open")
		open();
	if (id == "save")
		save();
	if (id == "save_as")
		save_as();

	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();


	if (id == "copy")
		copy();
	if (id == "paste")
		paste();
	if (id == "delete")
		data->delete_selection();

	if (id == "import_world_properties")
		import_world_properties();

	if (id == "create_objects")
		ed->set_mode(new ModeWorldCreateObject(ed->cur_mode));
	if (id == "terrain_create")
		ed->set_mode(new ModeWorldCreateTerrain(ed->cur_mode));
	if (id == "create-link")
		ed->set_mode(new ModeWorldCreateLink(ed->cur_mode));
	if (id == "create-light")
		ed->set_mode(new ModeWorldCreateLight(ed->cur_mode));
	if (id == "terrain_load")
		load_terrain();

	if (id == "objects-add-component")
		dialog->on_component_add();

	if (id == "mode_world")
		ed->set_mode(mode_world);
	if (id == "mode_world_camera")
		ed->set_mode(mode_world_camera);
	if (id == "mode_world_terrain")
		ed->set_mode(mode_world_terrain);

	if (id == "camscript_create")
		ed->set_mode(mode_world_camera);
	if (id == "camscript_load")
		if (storage->file_dialog(FD_CAMERAFLIGHT, false, true)) {
			if (mode_world_camera->data->load(storage->dialog_file_complete))
				ed->set_mode(mode_world_camera);
			else
				mode_world_camera->data->reset();
		}
	if (id == "edit_terrain_vertices")
		ed->set_mode(mode_world_terrain);
	if (id == "create_lightmap")
		ExecuteLightmapDialog();

	if (id == "own_figure")
		set_ego();
	if (id == "terrain_heightmap")
		apply_heightmap();

	if (id == "selection_properties")
		ExecutePropertiesDialog();

	if (id == "show_fx")
		toggle_show_effects();

	if (id == "select")
		set_mouse_action(MultiView::ACTION_SELECT);
	if (id == "translate")
		set_mouse_action(MultiView::ACTION_MOVE);
	if (id == "rotate")
		set_mouse_action(MultiView::ACTION_ROTATE);
}

#define MODEL_MAX_VERTICES	65536
vector tmv[MODEL_MAX_VERTICES*5],pmv[MODEL_MAX_VERTICES*5];
bool tvm[MODEL_MAX_VERTICES*5];

float WorldObject::hover_distance(MultiView::Window *win, const vec2 &mv, vector &tp, float &z) {
	Object *o = object;
	if (!o)
		return -1;
	int d = 0;//o->_detail_;
	if ((d<0)or(d>2))
		return -1;
	o->_matrix = matrix::translation(pos) * matrix::rotation(ang);
	for (int i=0;i<o->mesh[d]->vertex.num;i++) {
		tmv[i] = o->_matrix * o->mesh[d]->vertex[i];
		pmv[i] = win->project(tmv[i]);
	}
	float z_min=1;
	for (int mm=0;mm<o->material.num;mm++)
	for (int i=0;i<o->mesh[d]->sub[mm].num_triangles;i++) {
		vector a=pmv[o->mesh[d]->sub[mm].triangle_index[i*3  ]];
		vector b=pmv[o->mesh[d]->sub[mm].triangle_index[i*3+1]];
		vector c=pmv[o->mesh[d]->sub[mm].triangle_index[i*3+2]];
		if ((a.z<=0)or(b.z<=0)or(c.z<=0)or(a.z>=1)or(b.z>=1)or(c.z>=1))
			continue;
		float f,g;
		float az=a.z,bz=b.z,cz=c.z;
		a.z=b.z=c.z=0;
		GetBaryCentric(vector(mv.x,mv.y,0),a,b,c,f,g);
		if ((f>=0)and(g>=0)and(f+g<=1)) {
			float z=az + f*(bz-az) + g*(cz-az);
			if (z<z_min) {
				z_min=z;
				tp=tmv[o->mesh[d]->sub[mm].triangle_index[i*3  ]]
					+ f*(tmv[o->mesh[d]->sub[mm].triangle_index[i*3+1]]-tmv[o->mesh[d]->sub[mm].triangle_index[i*3  ]])
					+ g*(tmv[o->mesh[d]->sub[mm].triangle_index[i*3+2]]-tmv[o->mesh[d]->sub[mm].triangle_index[i*3  ]]);
			}
		}
	}
	z = z_min;
	return (z_min<1) ? 0 : -1;
}

bool WorldObject::in_rect(MultiView::Window *win, const rect &r) {
	Object *m = object;
	if (!m)
		return false;
	int d = 0;//m->_detail_;
	if ((d<0)or(d>2))
		return false;
	vector min, max;
	m->_matrix = matrix::translation(pos) * matrix::rotation(ang);
	for (int i=0;i<m->mesh[d]->vertex.num;i++) {
		tmv[i] = m->_matrix * m->mesh[d]->vertex[i];
		pmv[i] = win->project(tmv[i]);
		if (r.inside({pmv[i].x, pmv[i].y}))
			return true;
	}
	return false;
	for (int mm=0;mm<m->material.num;mm++)
	for (int i=0;i<m->mesh[d]->sub[mm].num_triangles;i++) {
		vector a=pmv[m->mesh[d]->sub[mm].triangle_index[i*3  ]];
		vector b=pmv[m->mesh[d]->sub[mm].triangle_index[i*3+1]];
		vector c=pmv[m->mesh[d]->sub[mm].triangle_index[i*3+2]];
		if ((a.z<=0)or(b.z<=0)or(c.z<=0)or(a.z>=1)or(b.z>=1)or(c.z>=1))
			continue;
		if (i==0)
			min = max = a;
		min._min(a);
		min._min(b);
		min._min(c);
		max._max(a);
		max._max(b);
		max._max(c);
	}
	return ((min.x>=r.x1)and(min.y>=r.y1)and(max.x<=r.x2)and(max.y<=r.y2));
}

bool WorldObject::overlap_rect(MultiView::Window *win, const rect &r) {
	return in_rect(win, r);
}

float WorldTerrain::hover_distance(MultiView::Window *win, const vec2 &mv, vector &tp, float &z) {
	//msg_db_f(format("IMOT index= %d",index).c_str(),3);
	Terrain *t = terrain;
	if (!t)
		return -1;
	float r = win->cam->radius * 100;
	vector a = win->unproject(vector(mv.x,mv.y,0));
	vector b = win->unproject(vector(mv.x,mv.y,0), win->cam->pos + win->get_direction() * r);
	CollisionData td;
	bool hit = t->trace(a - pos, b - pos, v_0, r, td, false);
	tp = td.p + pos;
	z = win->project(tp).z;
	return hit ? 0 : -1;
}

bool WorldTerrain::in_rect(MultiView::Window *win, const rect &r) {
	Terrain *t = terrain;
	vector min,max;
	for (int i=0;i<8;i++) {
		vector v = pos+vector((i%2)==0?t->min.x:t->max.x,((i/2)%2)==0?t->min.y:t->max.y,((i/4)%2)==0?t->min.z:t->max.z);
		vector p = win->project(v);
		if (i==0)
			min=max=p;
		min._min(p);
		max._max(p);
	}
	return ((min.x>=r.x1)and(min.y>=r.y1)and(max.x<=r.x2)and(max.y<=r.y2));
}
bool WorldTerrain::overlap_rect(MultiView::Window *win, const rect &r) {
	return in_rect(win, r);
}



bool ModeWorld::save() {
	for (auto &t: data->terrains) {
		if (t.filename.is_empty()) {
			if (!storage->file_dialog(FD_TERRAIN, true, true))
				return false;
			if (!t.save(storage->dialog_file_complete))
				return false;
		}
	}
	return storage->auto_save(data);
}



void ModeWorld::_new() {
	if (!ed->allow_termination())
		return;

	data->reset();
	optimize_view();
	ed->set_mode(mode_world);
}



void ModeWorld::on_draw() {
	//cur_cam->pos = multi_view->cam.pos;

	int num_ob = data->get_selected_objects();
	int num_te = data->get_selected_terrains();
	int num_cam = data->get_selected_cameras();
	int num_li = data->get_selected_lights();
	if (num_ob + num_te + num_cam + num_li > 0) {
		Array<string> ss;
		if (num_ob > 0)
			ss.add(format(_("%d objects"), num_ob));
		if (num_te > 0)
			ss.add(format(_("%d terrains"), num_te));
		if (num_cam > 0)
			ss.add(format(_("%d cameras"), num_cam));
		if (num_li > 0)
			ss.add(format(_("%d lights"), num_li));
		nix::set_shader(nix::Shader::default_2d);
		draw_str(10, 100, _("selected: ") + implode(ss, ", "));
	}
}



void ModeWorld::on_end() {
	if (world_dialog)
		delete world_dialog;
	world_dialog = NULL;

	ed->toolbar[hui::TOOLBAR_TOP]->reset();
	ed->toolbar[hui::TOOLBAR_TOP]->enable(false);
}


void ModeWorld::on_leave() {
	ed->set_side_panel(nullptr);
}



void DrawSelectionObject(const WorldObject &oo, float alpha, const color &c) {
	auto o = oo.object;
	if (!o)
		return;
	int d = 0;//o->_detail_;
	if ((d<0) or (d>3))
		return;
	nix::set_model_matrix(matrix::translation(oo.pos) * matrix::rotation(oo.ang));
	nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	for (int i=0;i<o->material.num;i++) {
		Array<nix::Texture*> tex;
		for (int j=0; j<o->material[i]->textures.num; j++)
			tex.add(NULL);
		nix::set_textures(tex);
		nix::set_material(color(alpha, 0, 0, 0), 0, 0, c);
		//o->just_draw(i, d);
		nix::draw_triangles(o->mesh[0]->sub[i].vertex_buffer);
	}
	nix::disable_alpha();
}

void DrawTerrainColored(Terrain *t, const color &c, float alpha, const vector &cam_pos) {
	nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);

	nix::set_material(color(alpha, 0, 0, 0), 0, 0, c);

	//nix::set_model_matrix(matrix::ID);
	//t->prepare_draw(cam_pos);
	nix::draw_triangles(t->vertex_buffer);


	nix::disable_alpha();
}



void ModeWorld::apply_lighting(MultiView::Window *win) {
	auto &m = data->meta_data;
	nix::set_fog(m.fog.mode, m.fog.start, m.fog.end, m.fog.density, m.fog.col);
	nix::enable_fog(m.fog.enabled);
	Array<nix::BasicLight> lights;
	for (auto &ll: data->lights) {
		nix::BasicLight l;
		l.proj = matrix::ID;
		l.theta = -1;
		l.radius = -1;
		l.pos = ll.pos;
		l.dir = ll.ang.ang2dir();
		if (ll.type == LightType::DIRECTIONAL) {
			//l.theta = pi;
			l.col = ll.col;
		} else if (ll.type == LightType::POINT) {
			l.radius = ll.radius;
			l.col = ll.col * (ll.radius * ll.radius / 100);
		} else if (ll.type == LightType::CONE) {
			l.radius = ll.radius;
			l.theta = ll.theta;
			l.col = ll.col * (ll.radius * ll.radius / 100);
		}
		l.harshness = ll.harshness;
		lights.add(l);
	}
	multi_view->ubo_light->update_array(lights);
	nix::bind_buffer(multi_view->ubo_light, 1);
	//win->set_shader(nix::Shader::default_3d, w->lights.num);
}

void ModeWorld::draw_background(MultiView::Window *win) {
	nix::clear_color(data->meta_data.background_color);
	/*NixSetZ(false,false);
	NixSetWire(false);
	NixSetColor(BackGroundColor);
	NixDraw2D(r_id, NixTargetRect, 0);
	NixSetWire(ed->multi_view_3d->wire_mode);
	NixSetZ(true,true);*/
}


void ModeWorld::draw_terrains(MultiView::Window *win) {
	foreachi(WorldTerrain &t, data->terrains, i) {
		if (!t.terrain)
			continue;
		if (t.view_stage < multi_view->view_stage)
			continue;
		nix::set_wire(multi_view->wire_mode);

		// prepare...
		t.terrain->prepare_draw(multi_view->cam.pos - t.pos);
		auto mat = t.terrain->material;
		try {
			mat->_prepare_shader((RenderPathType)1, ShaderVariant::DEFAULT);
		} catch (Exception &e) {
			msg_error(e.message());
		}

		auto s = mat->shader[0].get();
		//nix::set_shader(nix::Shader::default_3d);
		win->set_shader(s, data->lights.num);
		s->set_floats("pattern0", &t.terrain->texture_scale[0].x, 3);
		s->set_floats("pattern1", &t.terrain->texture_scale[1].x, 3);

		nix::set_model_matrix(matrix::translation(t.pos));
		nix::set_material(mat->albedo, mat->roughness, mat->metal, mat->emission);
		nix::set_textures(weak(mat->textures));
		nix::draw_triangles(t.terrain->vertex_buffer);

		nix::set_wire(false);

		//nix::set_shader(nix::Shader::default_3d);
		if (t.is_selected)
			DrawTerrainColored(t.terrain, Red, TSelectionAlpha, multi_view->cam.pos);
		if ((multi_view->hover.type == MVD_WORLD_TERRAIN) and (multi_view->hover.index == i))
			DrawTerrainColored(t.terrain, White, TMouseOverAlpha, multi_view->cam.pos);
	}
}

void ModeWorld::draw_objects(MultiView::Window *win) {
	//GodDraw();
	//MetaDrawSorted();
	//NixSetWire(false);
	nix::set_shader(nix::Shader::default_3d);
	nix::set_wire(multi_view->wire_mode);

	for (WorldObject &o: data->objects) {
		if (o.view_stage < multi_view->view_stage)
			continue;
		if (o.object) {
			nix::set_model_matrix(matrix::translation(o.pos) * matrix::rotation(o.ang));
			for (int i=0;i<o.object->material.num;i++) {
				auto mat = o.object->material[i];
				try {
					mat->_prepare_shader((RenderPathType)1, ShaderVariant::DEFAULT);
				} catch (Exception &e) {
					msg_error(e.message());
				}

				auto s = mat->shader[0].get();
				//nix::set_shader(nix::Shader::default_3d);
				win->set_shader(s, data->lights.num);

				//mat->shader = nullptr;
				nix::set_material(mat->albedo, mat->roughness, mat->metal, mat->emission);
				nix::set_textures(weak(mat->textures));
				nix::draw_triangles(o.object->mesh[0]->sub[i].vertex_buffer);
			}
			//o.object->draw(0, false, false);
			//o.object->_detail_ = 0;
		}
	}
	nix::set_wire(false);

	// object selection
	for (WorldObject &o: data->objects)
		if (o.is_selected)
			DrawSelectionObject(o, OSelectionAlpha, Red);
		else if (o.is_special)
			DrawSelectionObject(o, OSelectionAlpha, Green);
	if ((multi_view->hover.index >= 0) and (multi_view->hover.type == MVD_WORLD_OBJECT))
		DrawSelectionObject(data->objects[multi_view->hover.index], OSelectionAlpha, White);
	nix::disable_alpha();
	nix::set_model_matrix(matrix::ID);
}

void ModeWorld::draw_cameras(MultiView::Window *win) {
	for (auto &c: data->cameras) {
		if (c.view_stage < multi_view->view_stage)
			continue;

		set_color(color(1, 0.9f, 0.6f, 0.3f));
		set_line_width(scheme.LINE_WIDTH_THIN);
		if (c.is_selected) {
			//set_color(Red);
			set_line_width(scheme.LINE_WIDTH_MEDIUM);
		}
		auto q = quaternion::rotation_v(c.ang);
		float r = win->cam->radius * 0.1f;
		float rr = r * tan(c.fov / 2);
		vector ex = q * vector::EX * rr * 1.333f;
		vector ey = q * vector::EY * rr;
		vector ez = q * vector::EZ * r;
		draw_line(c.pos, c.pos + ez + ex + ey);
		draw_line(c.pos, c.pos + ez - ex + ey);
		draw_line(c.pos, c.pos + ez + ex - ey);
		draw_line(c.pos, c.pos + ez - ex - ey);
		draw_line(c.pos + ez + ex + ey, c.pos + ez - ex + ey);
		draw_line(c.pos + ez - ex + ey, c.pos + ez - ex - ey);
		draw_line(c.pos + ez - ex - ey, c.pos + ez + ex - ey);
		draw_line(c.pos + ez + ex - ey, c.pos + ez + ex + ey);
	}
}

void draw_tangent_circle(MultiView::Window *win, const vector &p, const vector &c, const vector &n, float r) {

	vector e1 = n.ortho();
	vector e2 = n ^ e1;
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
	draw_line(p, c + sin(w) * e1 + cos(w) * e2);
	draw_line(p, c - sin(w) * e1 - cos(w) * e2);
}

const float LIGHT_RADIUS_FACTOR_HI = 0.03f;
const float LIGHT_RADIUS_FACTOR_LO = 0.15f;

void ModeWorld::draw_lights(MultiView::Window *win) {
	for (auto &l: data->lights) {
		if (l.view_stage < multi_view->view_stage)
			continue;

		set_color(color(1, 0.9f, 0.6f, 0.3f));
		set_line_width(scheme.LINE_WIDTH_MEDIUM);
		if (l.is_selected) {
			//set_color(Red);
			set_line_width(scheme.LINE_WIDTH_THICK);
		}

		if (l.type == LightType::DIRECTIONAL) {
			draw_line(l.pos, l.pos + l.ang.ang2dir() * win->cam->radius * 0.1f);
		} else if (l.type == LightType::POINT) {
			//draw_circle(l.pos, win->get_direction(), l.radius);
			draw_circle(l.pos, win->get_direction(), l.radius * LIGHT_RADIUS_FACTOR_LO);
			draw_circle(l.pos, win->get_direction(), l.radius * LIGHT_RADIUS_FACTOR_HI);
		} else if (l.type == LightType::CONE) {
			draw_line(l.pos, l.pos + l.ang.ang2dir() * l.radius * LIGHT_RADIUS_FACTOR_LO);
			draw_circle(l.pos + l.ang.ang2dir() * l.radius*LIGHT_RADIUS_FACTOR_LO, l.ang.ang2dir(), l.radius * tan(l.theta) * LIGHT_RADIUS_FACTOR_LO);
			draw_circle(l.pos + l.ang.ang2dir() * l.radius*LIGHT_RADIUS_FACTOR_HI, l.ang.ang2dir(), l.radius * tan(l.theta) * LIGHT_RADIUS_FACTOR_HI);
			draw_tangent_circle(win, l.pos, l.pos + l.ang.ang2dir() * l.radius*LIGHT_RADIUS_FACTOR_LO, l.ang.ang2dir(), l.radius * tan(l.theta) * LIGHT_RADIUS_FACTOR_LO);
		}
	}
}

void ModeWorld::draw_links(MultiView::Window *win) {
	for (auto &l: data->links) {
		if (l.view_stage < multi_view->view_stage)
			continue;

		set_color(color(1, 0.9f, 0.6f, 0.3f));
		set_line_width(scheme.LINE_WIDTH_THIN);
		if (l.is_selected) {
			//set_color(Red);
			set_line_width(scheme.LINE_WIDTH_THICK);
		}
		draw_line(l.pos, data->objects[l.object[0]].pos);
		if (l.object[1] >= 0)
			draw_line(l.pos, data->objects[l.object[1]].pos);
		if (l.is_selected) {
			set_line_width(scheme.LINE_WIDTH_THICK);
			vector d = quaternion::rotation(l.ang) * vector::EZ * multi_view->cam.radius * 0.1;
			draw_line(l.pos - d, l.pos + d);
		}
	}
}

void ModeWorld::on_draw_win(MultiView::Window *win) {
	if (show_effects) {
		if (win->type == MultiView::VIEW_PERSPECTIVE)
			draw_background(win);
	}

	if (show_effects)
		apply_lighting(win);

	draw_terrains(win);
	draw_objects(win);
	draw_cameras(win);
	draw_lights(win);
	draw_links(win);


	nix::set_z(true,true);
	nix::enable_fog(false);
}



void ModeWorld::on_start() {
	ed->toolbar[hui::TOOLBAR_TOP]->set_by_id("world-toolbar");
	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("world-edit-toolbar");

	set_mouse_action(MultiView::ACTION_MOVE);

	data->update_data();
}

void ModeWorld::on_enter() {
	ed->toolbar[hui::TOOLBAR_TOP]->set_by_id("world-toolbar");
	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("world-edit-toolbar");

	dialog = new WorldObjectListPanel(this);
	ed->set_side_panel(dialog);
}

void ModeWorld::set_mouse_action(int mode) {
	mouse_action = mode;
	if (mode == MultiView::ACTION_MOVE)
		multi_view->set_mouse_action("ActionWorldMoveSelection", mode, false);
	else if (mode == MultiView::ACTION_ROTATE)
		multi_view->set_mouse_action("ActionWorldRotateObjects", mode, false);
	else
		multi_view->set_mouse_action("", mode, false);
}


void ModeWorld::on_update_menu() {
	ed->enable("undo", data->action_manager->undoable());
	ed->enable("redo", data->action_manager->redoable());

	ed->enable("copy", copyable());
	ed->enable("paste", pasteable());

	ed->check("show_fx", show_effects);

	ed->check("mode_world", mode_world->is_ancestor_of(ed->cur_mode) and !mode_world_camera->is_ancestor_of(ed->cur_mode) and !mode_world_terrain->is_ancestor_of(ed->cur_mode));
	ed->check("mode_world_camera", mode_world_camera->is_ancestor_of(ed->cur_mode));
	ed->check("mode_world_terrain", mode_world_terrain->is_ancestor_of(ed->cur_mode));

	ed->enable("select", multi_view->allow_mouse_actions);
	ed->enable("translate", multi_view->allow_mouse_actions);
	ed->enable("rotate", multi_view->allow_mouse_actions);
	ed->check("select", mouse_action == MultiView::ACTION_SELECT);
	ed->check("translate", mouse_action == MultiView::ACTION_MOVE);
	ed->check("rotate", mouse_action == MultiView::ACTION_ROTATE);
}



bool ModeWorld::open() {
	return ed->universal_open(FD_WORLD);
	/*if (!storage->open(data))
		return false;

	ed->set_mode(mode_world);
	optimize_view();
	return true;*/
}

void ModeWorld::ExecuteWorldPropertiesDialog() {
	if (world_dialog) {
		if (!world_dialog->active) {
			world_dialog->restart();
			world_dialog->show();
		}
		return;
	}

	world_dialog = new WorldPropertiesDialog(ed, true, data);
	world_dialog->show();
}



void ModeWorld::ExecutePropertiesDialog() {
	int num_o = data->get_selected_objects();
	int num_t = data->get_selected_terrains();

	if (num_o + num_t == 0) {
		// nothing selected -> world
		ExecuteWorldPropertiesDialog();
	} else if ((num_o == 0) and (num_t == 1)) {
		// single terrain -> terrain
		foreachi(WorldTerrain &t, data->terrains, i)
			if (t.is_selected)
				ExecuteTerrainPropertiesDialog(i);
	} else {
		ExecuteWorldPropertiesDialog();
	}
}





void ModeWorld::ExecuteTerrainPropertiesDialog(int index) {
	auto *dlg = new TerrainPropertiesDialog(ed, false, data, index);
	dlg->run();
	delete dlg;
}

void ModeWorld::ExecuteLightmapDialog() {
	auto *dlg = new LightmapDialog(ed, false, data);
	dlg->run();
	delete dlg;
}


bool ModeWorld::optimize_view() {
	multi_view->reset_view();
	vector min, max;
	data->get_bounding_box(min, max);
	multi_view->set_view_box(min, max);

	//ShowEffects = false;
	TerrainShowTextureLevel = -1;
	//TerrainsSelectable=false;
	return true;
}

void ModeWorld::load_terrain() {
	if (storage->file_dialog(FD_TERRAIN, false, true))
		data->add_terrain(storage->dialog_file_no_ending, multi_view->cam.pos);
}

void ModeWorld::set_ego() {
	if (data->get_selected_objects() != 1) {
		ed->set_message(_("Please select exactly one object!"));
		return;
	}
	foreachi(WorldObject &o, data->objects, i)
		if (o.is_selected)
			data->execute(new ActionWorldSetEgo(i));
}

void ModeWorld::toggle_show_effects() {
	show_effects = !show_effects;
	ed->update_menu();
	multi_view->force_redraw();
}


void ModeWorld::import_world_properties() {
	if (storage->file_dialog(FD_WORLD, false, false)) {
		DataWorld w;
		if (storage->load(storage->dialog_file_complete, &w, false))
			data->execute(new ActionWorldEditData(w.meta_data));
		else
			ed->error_box(_("World could not be loaded correctly!"));
	}
}

void ModeWorld::apply_heightmap() {
	if (data->get_selected_terrains() == 0) {
		ed->set_message(_("No terrain selected!"));
		return;
	}
	auto *dlg = new TerrainHeightmapDialog(ed, false, data);
	dlg->run();
	delete dlg;
}







void ModeWorld::copy() {
	data->copy(temp_objects, temp_terrains, temp_cameras, temp_lights);

	on_update_menu();
	ed->set_message(format(_("copied %d objects, %d terrains, %d lights"), temp_objects.num, temp_terrains.num, temp_lights.num));
}

void ModeWorld::paste() {
	data->paste(temp_objects, temp_terrains, temp_cameras, temp_lights);
	ed->set_message(format(_("added %d objects, %d terrains, %d lights"), temp_objects.num, temp_terrains.num, temp_lights.num));
}

bool ModeWorld::copyable() {
	return (data->get_selected_objects() + data->get_selected_terrains() + data->get_selected_lights()) > 0;
}

bool ModeWorld::pasteable() {
	return (temp_objects.num + temp_terrains.num + temp_lights.num) > 0;
}

void ModeWorld::on_set_multi_view() {
	multi_view->clear_data(data);

	multi_view->add_data(MVD_WORLD_OBJECT,
			data->objects,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
	multi_view->add_data(MVD_WORLD_TERRAIN,
			data->terrains,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
	multi_view->add_data(MVD_WORLD_LIGHT,
			data->lights,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE | MultiView::FLAG_DRAW);
	multi_view->add_data(MVD_WORLD_CAMERA,
			data->cameras,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE | MultiView::FLAG_DRAW);
	multi_view->add_data(MVD_WORLD_LINK,
			data->links,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE | MultiView::FLAG_DRAW);
}
