//
// Created by Michael Ankele on 2025-04-30.
//

#include "MaterialPanel.h"
#include "../action/ActionMaterialEditAppearance.h"
#include <Session.h>
#include <helper/ResourceManager.h>
#include <lib/image/image.h>
#include <lib/xhui/controls/Image.h>
#include <lib/xhui/controls/ListView.h>


string file_secure(const Path &filename);

MaterialPanel::MaterialPanel(ModeMaterial *_mode) : Node<xhui::Panel>("") {
	from_resource("material-panel");
	data = _mode->data;

	auto tex_list = (xhui::ListView*)get_control("textures");
	tex_list->column_factories[1].f_create = [](const string& id) {
		return new xhui::Image(id, "");
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

	//delete popup_materials;
}


ModeMaterial *MaterialPanel::mode_material() {
	return data->session->mode_material;
}

Image* preview_texture(Session* s, const Path& filename) {
	static Image* dummy = nullptr;
	static base::map<Path, Image*> previews;

	if (previews.contains(filename))
		return previews[filename];

	const auto path = s->resource_manager->find_absolute_texture_path(filename);
	if (path.is_empty()) {
		if (!dummy)
			dummy = new Image(40, 40, White);
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
		string id = format("image:material-texture[%d]", i);
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
	fill_texture_list();
}
