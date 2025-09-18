//
// Created by Michael Ankele on 2025-04-30.
//

#include "MaterialPanel.h"
#include "MaterialPassPanel.h"
#include "../action/ActionMaterialEditAppearance.h"
#include <Session.h>
#include <helper/ResourceManager.h>
#include <storage/Storage.h>
#include <lib/image/image.h>
#include <lib/xhui/Resource.h>
#include <lib/xhui/Menu.h>
#include <lib/xhui/controls/Image.h>
#include <lib/xhui/controls/ListView.h>
#include <lib/yrenderer/TextureManager.h>
#include <storage/format/Format.h>
#include <view/DocumentSession.h>


string file_secure(const Path &filename);

MaterialPanel::MaterialPanel(ModeMaterial *_mode) : Node<xhui::Panel>("") {
	from_resource("material-panel");
	data = _mode->data;

	popup_textures = xhui::create_resource_menu("model-texture-list-popup");

	auto tex_list = (xhui::ListView*)get_control("textures");
	tex_list->column_factories[1].f_create = [](const string& id) {
		return new xhui::Image(id, "");
	};


	auto pass_list = (xhui::ListView*)get_control("passes");
	pass_list->column_factories[0].f_create = [this](const string& id) {
		return new MaterialPassPanel(this, data, 0);
	};
	pass_list->column_factories[0].f_set = [this](xhui::Control* c, const string& t) {
		int i = t._int();
		reinterpret_cast<MaterialPassPanel*>(c)->update(i);
	};
	pass_list->column_factories[0].f_select = [this](xhui::Control* c, bool selected) {
		reinterpret_cast<MaterialPassPanel*>(c)->set_selected(selected);
	};

	event("albedo", [this] {
		apply_queue_depth ++;
		auto a = data->appearance;
		a.albedo = get_color("albedo");
		data->execute(new ActionMaterialEditAppearance(a));
		apply_queue_depth --;
	});
	event("emission", [this] {
		apply_queue_depth ++;
		auto a = data->appearance;
		a.emissive = get_color("emission");
		data->execute(new ActionMaterialEditAppearance(a));
		apply_queue_depth --;
	});
	event("metal", [this] {
		apply_queue_depth ++;
		auto a = data->appearance;
		a.metal = get_float("metal");
		data->execute(new ActionMaterialEditAppearance(a));
		set_float("slider-metal", a.metal);
		apply_queue_depth --;
	});
	event("slider-metal", [this] {
		apply_queue_depth ++;
		auto a = data->appearance;
		a.metal = get_float("slider-metal");
		data->execute(new ActionMaterialEditAppearance(a));
		set_float("metal", a.metal);
		apply_queue_depth --;
	});
	event("roughness", [this] {
		apply_queue_depth ++;
		auto a = data->appearance;
		a.roughness = get_float("roughness");
		data->execute(new ActionMaterialEditAppearance(a));
		set_float("slider-roughness", a.roughness);
		apply_queue_depth --;
	});
	event("slider-roughness", [this] {
		apply_queue_depth ++;
		auto a = data->appearance;
		a.roughness = get_float("slider-roughness");
		data->execute(new ActionMaterialEditAppearance(a));
		set_float("roughness", a.roughness);
		apply_queue_depth --;
	});
	event("cast-shadows", [this] {
		apply_queue_depth ++;
		auto a = data->appearance;
		a.cast_shadow = is_checked("cast-shadows");
		data->execute(new ActionMaterialEditAppearance(a));
		apply_queue_depth --;
	});
	event("texture-level-add", [this] { on_texture_level_add(); });
	event_x("textures", xhui::event_id::RightButtonDown, [this] { on_textures_right_click(); });
	event("texture-level-delete", [this] { on_texture_level_delete(); });
	event("texture-level-clear", [this] { on_texture_level_clear(); });
	event("texture-level-load", [this] { on_texture_level_load(); });

	data->out_changed >> create_sink([this] {
		if (apply_queue_depth == 0)
			load_data();
	});

	load_data();
	apply_queue_depth = 0;
}

MaterialPanel::~MaterialPanel() {
	mode_material()->unsubscribe(this);
	data->unsubscribe(this);
}


ModeMaterial *MaterialPanel::mode_material() {
	return data->doc->mode_material;
}

Image* preview_texture(Session* s, const Path& filename) {
	static Image* empty = nullptr;
	static Image* dummy = nullptr;
	static base::map<Path, Image*> previews;

	if (!empty)
		empty = new Image(40, 40, White);
	if (!dummy) {
		dummy = new Image(40, 40, Red);
		// TODO "X" or "?"
	}

	if (filename.is_empty())
		return empty;

	if (previews.contains(filename))
		return previews[filename];

	const auto path = s->resource_manager->texture_manager->find_absolute_texture_path(filename);
	if (path.is_empty()) {
		return dummy;
	} else {
		auto im = Image::load(path);
		auto preview = im->scale(40, 40);
		previews.set(filename, preview);
		delete im;
		return preview;
	}
}

void MaterialPanel::fill_texture_list() {
	reset("textures");
	for (int i=0;i<data->appearance.texture_files.num;i++) {
		string id = format("image:texture-icon[%s]", data->appearance.texture_files[i]);
		auto icon = preview_texture(data->session, data->appearance.texture_files[i]);
		xhui::set_image(id, *icon);
		string ext = "";//format(" (%dx%d)", img->width, img->height);
		add_string("textures", format("Tex[%d]\\%s\\%s", i, id, (file_secure(data->appearance.texture_files[i]) + ext)));
	}
}

// data -> GUI
void MaterialPanel::load_data() {
	set_color("albedo", data->appearance.albedo);
	set_color("emission", data->appearance.emissive);
	set_float("roughness", data->appearance.roughness);
	set_float("slider-roughness", data->appearance.roughness);
	set_float("metal", data->appearance.metal);
	set_float("slider-metal", data->appearance.metal);
	check("cast-shadows", data->appearance.cast_shadow);
	fill_texture_list();


	reset("passes");
	for (int i=0;i<data->appearance.passes.num;i++) {
		add_string("passes", str(i));
	}
}


void MaterialPanel::on_texture_level_add() {
	if (data->appearance.texture_files.num >= MATERIAL_MAX_TEXTURES) {
		data->session->error(format("Only %d texture levels allowed!", MATERIAL_MAX_TEXTURES));
		return;
	}

	auto a = data->appearance;
	a.texture_files.add("");
	data->execute(new ActionMaterialEditAppearance(a));
}

void MaterialPanel::on_texture_level_load() {
	int index = get_int("textures");
	if (index >= 0)
		data->session->storage->file_dialog(FD_TEXTURE, false, true).then([this, index] (const auto& p) {
			auto a = data->appearance;
			a.texture_files[index] = p.relative;
			data->execute(new ActionMaterialEditAppearance(a));
		});
}

void MaterialPanel::on_texture_level_delete() {
	int index = get_int("textures");
	if (index >= 0) {
		if (data->appearance.texture_files.num <= 1) {
			data->session->error("At least one texture level has to exist!");
			return;
		}

		auto a = data->appearance;
		a.texture_files.erase(index);
		data->execute(new ActionMaterialEditAppearance(a));
	}
}

void MaterialPanel::on_texture_level_clear() {
	int index = get_int("textures");
	if (index >= 0) {
		auto a = data->appearance;
		a.texture_files[index] = "";
		data->execute(new ActionMaterialEditAppearance(a));
	}
}

void MaterialPanel::on_textures_right_click() {
	int n = get_int("textures");
	if (n >= 0) {
		//mode_mesh()->set_current_texture_level(n);
	}
	popup_textures->enable("texture-level-delete", n>=0);
	popup_textures->enable("texture-level-clear", n>=0);
	popup_textures->enable("texture-level-load", n>=0);
	popup_textures->enable("texture-level-save", n>=0);
	popup_textures->enable("texture-level-scale", n>=0);
	popup_textures->open_popup(this);
}
