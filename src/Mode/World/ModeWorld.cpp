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
#include "../../x/camera.h"
#include "../../x/world.h"
#include "../../x/material.h"
#include "../../x/model.h"
#include "../../x/object.h"
#include "../../x/terrain.h"
#include "Dialog/TerrainPropertiesDialog.h"
#include "Dialog/TerrainHeightmapDialog.h"
#include "Dialog/LightmapDialog.h"
#include "Dialog/WorldPropertiesDialog.h"
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


const bool LIST_SHOW_SCRIPTS = false;



class WorldObjectListPanel : public hui::Panel {
public:
	ModeWorld *world;
	DataWorld *data;

	struct Index {
		int type, index;
	};
	Array<Index> list_indices;
	int editing;
	bool allow_sel_change_signal;
	hui::Menu *popup;

	WorldObjectListPanel(ModeWorld *w) {
		from_resource("world-object-list-dialog");

		popup = hui::CreateResourceMenu("world-object-list-popup");

		world = w;
		data = world->data;
		editing = -1;
		allow_sel_change_signal = true;
		event_x("list", "hui:select", [=]{ on_list_select(); });
		event_x("list", "hui:right-button-down", [=]{ on_list_right_click(); });
		event("light-enabled", [=]{ on_change(); });
		event("light-type", [=]{ on_change(); });
		event("light-col", [=]{ on_change(); });
		event("light-harshness", [=]{ on_change(); });
		event("light-radius", [=]{ on_change(); });
		event("link-type", [=]{ on_change(); });
		event("link-friction", [=]{ on_change(); });
		event("cam-fov", [=]{ on_change(); });
		event("cam-min-depth", [=]{ on_change(); });
		event("cam-max-depth", [=]{ on_change(); });
		event("cam-exposure", [=]{ on_change(); });
		event("script-edit", [=]{ on_script_edit(); });

		fill_list();

		data->subscribe(this, [=]{ fill_list(); }, data->MESSAGE_CHANGE);
		w->multi_view->subscribe(this, [=] {
			if (allow_sel_change_signal)
				selection_from_world();
		}, w->multi_view->MESSAGE_SELECTION_CHANGE);
	}
	~WorldObjectListPanel() {
		world->multi_view->unsubscribe(this);
		data->unsubscribe(this);
		delete popup;
	}

	void fill_list() {
		set_editing(-1);
		reset("list");
		list_indices.clear();

		foreachi (auto &c, data->cameras, i) {
			add_string("list", "Camera\\cam");
			list_indices.add({MVD_WORLD_CAMERA, i});
		}
		foreachi (auto &l, data->lights, i) {
			add_string("list", "Light\\" + light_type(l.type));
			list_indices.add({MVD_WORLD_LIGHT, i});
		}
		if (LIST_SHOW_SCRIPTS) {
		foreachi (auto &s, data->meta_data.scripts, i) {
			add_string("list", "Script\\" + s.filename.str());
			list_indices.add({MVD_WORLD_SCRIPT, i});
		}
		}
		foreachi (auto &t, data->terrains, i) {
			add_string("list", "Terrain\\" + t.filename.str());
			list_indices.add({MVD_WORLD_TERRAIN, i});
		}
		foreachi (auto &o, data->objects, i) {
			add_string("list", "Object\\" + ((o.name == "") ? o.filename.str() : o.name));
			list_indices.add({MVD_WORLD_OBJECT, i});
		}
		foreachi (auto &l, data->links, i) {
			add_string("list", "Link\\" + link_type_canonical(l.type));
			list_indices.add({MVD_WORLD_LINK, i});
		}
		selection_from_world();
	}

	void selection_from_world() {
		Array<int> sel;

		foreachi (auto &ii, list_indices, i) {
			if (ii.type == MVD_WORLD_OBJECT) {
				auto &o = data->objects[ii.index];
				if (o.is_selected)
					sel.add(i);
			} else if (ii.type == MVD_WORLD_TERRAIN) {
				auto &t = data->terrains[ii.index];
				if (t.is_selected)
					sel.add(i);
			} else if (ii.type == MVD_WORLD_SCRIPT) {
				auto &s = data->meta_data.scripts[ii.index];
				//if (s.is_selected)
				//	sel.add(i);
			} else if (ii.type == MVD_WORLD_LIGHT) {
				auto &l = data->lights[ii.index];
				if (l.is_selected)
					sel.add(i);
			} else if (ii.type == MVD_WORLD_LINK) {
				auto &l = data->links[ii.index];
				if (l.is_selected)
					sel.add(i);
			} else if (ii.type == MVD_WORLD_CAMERA) {
				auto &c = data->cameras[ii.index];
				if (c.is_selected)
					sel.add(i);
			}
		}
		set_selection("list", sel);
		if (sel.num == 1)
			set_editing(sel[0]);
		else
			set_editing(-1);
	}

	void on_list_right_click() {
		int row = hui::GetEvent()->row;
		popup->enable("delete", row >= 0);
		popup->open_popup(win);
	}

	void on_list_select() {
		auto sel = get_selection("list");
		selection_to_world(sel);

		if (sel.num == 1)
			set_editing(sel[0]);
		else
			set_editing(-1);
	}

	void selection_to_world(const Array<int> &sel) {
		allow_sel_change_signal = false;

		world->multi_view->select_none();

		for (int s: sel) {
		auto &ii = list_indices[s];
			if (ii.type == MVD_WORLD_OBJECT) {
				auto &o = data->objects[ii.index];
				o.is_selected = true;
			} else if (ii.type == MVD_WORLD_TERRAIN) {
				auto &t = data->terrains[ii.index];
				t.is_selected = true;
			} else if (ii.type == MVD_WORLD_SCRIPT) {
				auto &s = data->meta_data.scripts[ii.index];
			} else if (ii.type == MVD_WORLD_LIGHT) {
				auto &l = data->lights[ii.index];
				l.is_selected = true;
			} else if (ii.type == MVD_WORLD_LINK) {
				auto &l = data->links[ii.index];
				l.is_selected = true;
			} else if (ii.type == MVD_WORLD_CAMERA) {
				auto &c = data->cameras[ii.index];
				c.is_selected = true;
			}
		}
		allow_sel_change_signal = true;
	}
	void set_editing(int s) {
		editing = s;
		if (editing < 0) {
			hide_control("g-object", true);
			hide_control("g-terrain", true);
			hide_control("g-light", true);
			hide_control("g-camera", true);
			hide_control("g-script", true);
			hide_control("g-location", true);
			return;
		}

		auto &ii = list_indices[editing];
		hide_control("g-object", ii.type != MVD_WORLD_OBJECT);
		hide_control("g-terrain", ii.type != MVD_WORLD_TERRAIN);
		hide_control("g-light", ii.type != MVD_WORLD_LIGHT);
		hide_control("g-link", ii.type != MVD_WORLD_LINK);
		hide_control("g-camera", ii.type != MVD_WORLD_CAMERA);
		hide_control("g-script", ii.type != MVD_WORLD_SCRIPT);
		hide_control("g-location", ii.type == MVD_WORLD_SCRIPT);

		if (ii.type == MVD_WORLD_OBJECT) {
			auto &o = data->objects[ii.index];
			set_string("ob-name", o.name);
			set_string("ob-kind", o.filename.str());
			set_float("pos-x", o.pos.x);
			set_float("pos-y", o.pos.y);
			set_float("pos-z", o.pos.z);
			set_float("ang-x", o.ang.x * 180.0f / pi);
			set_float("ang-y", o.ang.y * 180.0f / pi);
			set_float("ang-z", o.ang.z * 180.0f / pi);
		} else if (ii.type == MVD_WORLD_TERRAIN) {
			auto &t = data->terrains[ii.index];
			set_string("terrain-file", t.filename.str());
			set_int("terrain-num-x", t.terrain->num_x);
			set_int("terrain-num-z", t.terrain->num_z);
			set_float("terrain-pattern-x", t.terrain->pattern.x);
			set_float("terrain-pattern-z", t.terrain->pattern.z);
			set_float("pos-x", t.pos.x);
			set_float("pos-y", t.pos.y);
			set_float("pos-z", t.pos.z);
			set_float("ang-x", 0);
			set_float("ang-y", 0);
			set_float("ang-z", 0);
		} else if (ii.type == MVD_WORLD_SCRIPT) {
			auto &s = data->meta_data.scripts[ii.index];
			set_string("script-file", s.filename.str());
			set_string("script-class", "???");
			reset("script-variables");
			for (auto &v: s.variables)
				add_string("script-variables", v.name + "\\" + v.type + "\\" + v.value);
		} else if (ii.type == MVD_WORLD_LIGHT) {
			auto &l = data->lights[ii.index];
			check("light-enabled", l.enabled);
			set_color("light-col", l.col);
			set_int("light-type", (int)l.type);
			set_float("light-radius", l.radius);
			set_float("light-harshness", l.harshness);
			set_float("pos-x", l.pos.x);
			set_float("pos-y", l.pos.y);
			set_float("pos-z", l.pos.z);
			set_float("ang-x", l.ang.x * 180.0f / pi);
			set_float("ang-y", l.ang.y * 180.0f / pi);
			set_float("ang-z", l.ang.z * 180.0f / pi);
		} else if (ii.type == MVD_WORLD_CAMERA) {
			auto &c = data->cameras[ii.index];
			set_float("cam-fov", c.fov * 180.0f / pi);
			set_float("cam-min-depth", c.min_depth);
			set_float("cam-max-depth", c.max_depth);
			set_float("cam-exposure", c.exposure);
			set_float("pos-x", c.pos.x);
			set_float("pos-y", c.pos.y);
			set_float("pos-z", c.pos.z);
			set_float("ang-x", c.ang.x * 180.0f / pi);
			set_float("ang-y", c.ang.y * 180.0f / pi);
			set_float("ang-z", c.ang.z * 180.0f / pi);
		} else if (ii.type == MVD_WORLD_LINK) {
			auto &l = data->links[ii.index];
			set_int("link-type", (int)l.type);
			set_float("link-friction", l.friction);
			set_float("pos-x", l.pos.x);
			set_float("pos-y", l.pos.y);
			set_float("pos-z", l.pos.z);
			set_float("ang-x", l.ang.x * 180.0f / pi);
			set_float("ang-y", l.ang.y * 180.0f / pi);
			set_float("ang-z", l.ang.z * 180.0f / pi);
		}
	}

	void on_change() {
		if (editing < 0)
			return;
		auto &ii = list_indices[editing];
		if (ii.type == MVD_WORLD_LIGHT) {
			auto &l = data->lights[ii.index];
			l.type = (LightType)get_int("light-type");
			l.enabled = is_checked("light-enabled");
			l.harshness = get_float("light-harshness");
			l.radius = get_float("light-radius");
			l.col = get_color("light-col");
			world->multi_view->force_redraw();
		} else if (ii.type == MVD_WORLD_CAMERA) {
			auto &c = data->cameras[ii.index];
			c.fov = get_float("cam-fov") * pi / 180.0f;
			c.min_depth = get_float("cam-min-depth");
			c.max_depth = get_float("cam-max-depth");
			c.exposure = get_float("cam-exposure");
			world->multi_view->force_redraw();
		} else if (ii.type == MVD_WORLD_LINK) {
			auto &l = data->links[ii.index];
			l.type = (LinkType)get_int("link-type");
			l.friction = get_float("link-friction");
			world->multi_view->force_redraw();
		}
	}

	void on_script_edit() {
		if (editing < 0)
			return;
		auto &ii = list_indices[editing];
		if (ii.type == MVD_WORLD_SCRIPT) {
			auto filename = Kaba::config.directory << data->meta_data.scripts[ii.index].filename;
			int r = system(("sgribthmaker '" + filename.str() + "'").c_str());
			//hui::OpenDocument(filename);
		}
	}
};


ModeWorld::ModeWorld() :
	Mode<DataWorld>("World", NULL, new DataWorld, ed->multi_view_3d, "menu_world") {
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

float WorldObject::hover_distance(MultiView::Window *win, const vector &mv, vector &tp, float &z) {
	Object *o = object;
	if (!o)
		return -1;
	int d = o->_detail_;
	if ((d<0)or(d>2))
		return -1;
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
		GetBaryCentric(mv,a,b,c,f,g);
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
	int d = m->_detail_;
	if ((d<0)or(d>2))
		return false;
	vector min, max;
	for (int i=0;i<m->mesh[d]->vertex.num;i++) {
		tmv[i] = m->_matrix * m->mesh[d]->vertex[i];
		pmv[i] = win->project(tmv[i]);
		if (r.inside(pmv[i].x, pmv[i].y))
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

float WorldTerrain::hover_distance(MultiView::Window *win, const vector &mv, vector &tp, float &z) {
	//msg_db_f(format("IMOT index= %d",index).c_str(),3);
	Terrain *t = terrain;
	if (!t)
		return -1;
	float r = win->cam->radius * 100;
	vector a = win->unproject(mv);
	vector b = win->unproject(mv, win->cam->pos + win->get_direction() * r);
	TraceData td;
	bool hit = t->trace(a, b, v_0, r, td, false);
	tp = td.point;
	z = win->project(tp).z;
	return hit ? 0 : -1;
}

bool WorldTerrain::in_rect(MultiView::Window *win, const rect &r) {
	Terrain *t = terrain;
	vector min,max;
	for (int i=0;i<8;i++) {
		vector v=t->pos+vector((i%2)==0?t->min.x:t->max.x,((i/2)%2)==0?t->min.y:t->max.y,((i/4)%2)==0?t->min.z:t->max.z);
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
	cur_cam->pos = multi_view->cam.pos;

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
		nix::SetShader(nix::default_shader_2d);
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



void DrawSelectionObject(Model *o, float alpha, const color &c) {
	if (!o)
		return;
	int d = o->_detail_;
	if ((d<0) or (d>3))
		return;
	nix::SetWorldMatrix(o->_matrix);
	nix::SetAlpha(ALPHA_MATERIAL);
	for (int i=0;i<o->material.num;i++) {
		Array<nix::Texture*> tex;
		for (int j=0; j<o->material[i]->textures.num; j++)
			tex.add(NULL);
		nix::SetTextures(tex);
		nix::SetMaterial(Black, color(alpha, 0, 0, 0), Black, 0, c);
		//o->just_draw(i, d);
		nix::DrawTriangles(o->mesh[0]->sub[i].vertex_buffer);
	}
	nix::SetAlpha(ALPHA_NONE);
}

void DrawTerrainColored(Terrain *t, const color &c, float alpha) {
	nix::SetAlpha(ALPHA_MATERIAL);

	nix::SetMaterial(Black, color(alpha, 0, 0, 0), Black, 0, c);

	nix::SetWorldMatrix(matrix::ID);
	t->draw();
	nix::DrawTriangles(t->vertex_buffer);


	nix::SetAlpha(ALPHA_NONE);
}



void apply_lighting(DataWorld *w) {
	auto &m = w->meta_data;
	nix::SetFog(m.fog.mode, m.fog.start, m.fog.end, m.fog.density, m.fog.col);
	nix::EnableFog(m.fog.enabled);
	for (auto &ll: w->lights)
		if (ll.type == LightType::DIRECTIONAL) {
			ed->multi_view_3d->set_light(ll.ang.ang2dir(), ll.col, ll.harshness);
		}
}

void draw_background(DataWorld *w) {
	nix::ResetToColor(w->meta_data.background_color);
	/*NixSetZ(false,false);
	NixSetWire(false);
	NixSetColor(BackGroundColor);
	NixDraw2D(r_id, NixTargetRect, 0);
	NixSetWire(ed->multi_view_3d->wire_mode);
	NixSetZ(true,true);*/
}


void ModeWorld::on_draw_win(MultiView::Window *win) {
	if (show_effects) {
		if (win->type == MultiView::VIEW_PERSPECTIVE)
			draw_background(data);
		apply_lighting(data);
	}

// terrain
	nix::SetWire(multi_view->wire_mode);
	nix::SetShader(nix::default_shader_3d);
	foreachi(WorldTerrain &t, data->terrains, i) {
		if (!t.terrain)
			continue;
		if (t.view_stage < multi_view->view_stage)
			continue;

		// prepare...
		t.terrain->draw();

		auto mat = t.terrain->material;
		nix::SetMaterial(mat->ambient, mat->diffuse, mat->specular, mat->shininess, mat->emission);
		nix::SetTextures(mat->textures);
		nix::DrawTriangles(t.terrain->vertex_buffer);

		if (t.is_selected)
			DrawTerrainColored(t.terrain, Red, TSelectionAlpha);
		if ((multi_view->hover.type == MVD_WORLD_TERRAIN) and (multi_view->hover.index == i))
			DrawTerrainColored(t.terrain, White, TMouseOverAlpha);
	}

// objects (models)
		//GodDraw();
		//MetaDrawSorted();
		//NixSetWire(false);

	for (WorldObject &o: data->objects) {
		if (o.view_stage < multi_view->view_stage)
			continue;
		if (o.object) {
			nix::SetWorldMatrix(matrix::translation(o.pos) * matrix::rotation(o.ang));
			for (int i=0;i<o.object->material.num;i++) {
				auto mat = o.object->material[i];
				mat->shader = NULL;
				nix::SetMaterial(mat->ambient, mat->diffuse, mat->specular, mat->shininess, mat->emission);
				nix::SetTextures(mat->textures);
				nix::DrawTriangles(o.object->mesh[0]->sub[i].vertex_buffer);
			}
			//o.object->draw(0, false, false);
			o.object->_detail_ = 0;
		}
	}
	nix::SetWire(false);

	// object selection
	for (WorldObject &o: data->objects)
		if (o.is_selected)
			DrawSelectionObject(o.object, OSelectionAlpha, Red);
		else if (o.is_special)
			DrawSelectionObject(o.object, OSelectionAlpha, Green);
	if ((multi_view->hover.index >= 0) and (multi_view->hover.type == MVD_WORLD_OBJECT))
		DrawSelectionObject(data->objects[multi_view->hover.index].object, OSelectionAlpha, White);
	nix::SetAlpha(ALPHA_NONE);
	nix::SetWorldMatrix(matrix::ID);


	// lights
	for (auto &l: data->lights) {
		if (l.view_stage < multi_view->view_stage)
			continue;

		set_color(color(1, 0.9f, 0.6f, 0.3f));
		set_line_width(scheme.LINE_WIDTH_MEDIUM);
		if (l.is_selected) {
			//set_color(Red);
			set_line_width(scheme.LINE_WIDTH_THICK);
		}
		draw_line(l.pos, l.pos + l.ang.ang2dir() * win->cam->radius * 0.1f);
	}

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

	for (auto &l: data->links) {
		if (l.view_stage < multi_view->view_stage)
			continue;

		set_color(color(1, 0.9f, 0.6f, 0.3f));
		set_line_width(scheme.LINE_WIDTH_THIN);
		if (l.is_selected) {
			//set_color(Red);
			set_line_width(5);
			set_line_width(scheme.LINE_WIDTH_MEDIUM);
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

	nix::SetZ(true,true);
	nix::EnableFog(false);
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
	if (!storage->open(data))
		return false;

	ed->set_mode(mode_world);
	optimize_view();
	return true;
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
	data->copy(temp_objects, temp_terrains);

	on_update_menu();
	ed->set_message(format(_("copied %d objects, %d terrains"), temp_objects.num, temp_terrains.num));
}

void ModeWorld::paste() {
	data->paste(temp_objects, temp_terrains);
	ed->set_message(format(_("added %d objects, %d terrains"), temp_objects.num, temp_terrains.num));
}

bool ModeWorld::copyable() {
	return (data->get_selected_objects() + data->get_selected_terrains()) > 0;
}

bool ModeWorld::pasteable() {
	return (temp_objects.num + temp_terrains.num) > 0;
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
