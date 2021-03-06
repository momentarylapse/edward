/*
 * WorldObjectListPanel.cpp
 *
 *  Created on: Dec 28, 2020
 *      Author: michi
 */


#include "WorldObjectListPanel.h"
#include "../ModeWorld.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Data/World/DataCamera.h"
#include "../../../Data/World/WorldLink.h"
#include "../../../Data/World/WorldLight.h"
#include "../../../Data/World/WorldObject.h"
#include "../../../Data/World/WorldTerrain.h"
#include "../../../Data/World/WorldCamera.h"
#include "../../../Storage/Storage.h"
#include "../../../Action/World/Object/ActionWorldEditObject.h"
#include "../../../MultiView/MultiView.h"
#include "../../../y/Object.h"
#include "../../../y/Terrain.h"
#include "../../../lib/kaba/kaba.h"
#include "../../../Edward.h"



const bool LIST_SHOW_SCRIPTS = false;


WorldObjectListPanel::WorldObjectListPanel(ModeWorld *w) {
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
	event("find-script", [=]{ on_find_script(); });

	fill_list();

	data->subscribe(this, [=]{ fill_list(); }, data->MESSAGE_CHANGE);
	w->multi_view->subscribe(this, [=] {
		if (allow_sel_change_signal)
			selection_from_world();
	}, w->multi_view->MESSAGE_SELECTION_CHANGE);
}

WorldObjectListPanel::~WorldObjectListPanel() {
	world->multi_view->unsubscribe(this);
	data->unsubscribe(this);
	delete popup;
}

void WorldObjectListPanel::on_find_script() {
	if (!storage->file_dialog(FD_SCRIPT, false, true))
		return;

	auto &ii = list_indices[editing];
	if (ii.type != MVD_WORLD_OBJECT)
		return;
	auto &o = data->objects[ii.index];
	WorldObject oo = o;
	oo.script = storage->dialog_file;
	auto a = new ActionWorldEditObject(ii.index, oo);
	data->execute(a);
}

void WorldObjectListPanel::fill_list() {
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

void WorldObjectListPanel::selection_from_world() {
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

void WorldObjectListPanel::on_list_right_click() {
	int row = hui::GetEvent()->row;
	popup->enable("delete", row >= 0);
	popup->open_popup(win);
}

void WorldObjectListPanel::on_list_select() {
	auto sel = get_selection("list");
	selection_to_world(sel);

	if (sel.num == 1)
		set_editing(sel[0]);
	else
		set_editing(-1);
}

void WorldObjectListPanel::selection_to_world(const Array<int> &sel) {
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
void WorldObjectListPanel::set_editing(int s) {
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
		set_string("ob-script", o.script.str());
		if (o.script.is_empty())
			set_string("ob-script", o.object->_template->script_filename.str());
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

void WorldObjectListPanel::on_change() {
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

void WorldObjectListPanel::on_script_edit() {
	if (editing < 0)
		return;
	auto &ii = list_indices[editing];
	if (ii.type == MVD_WORLD_SCRIPT) {
		auto filename = kaba::config.directory << data->meta_data.scripts[ii.index].filename;
		int r = system(("sgribthmaker '" + filename.str() + "'").c_str());
		//hui::OpenDocument(filename);
	}
}


