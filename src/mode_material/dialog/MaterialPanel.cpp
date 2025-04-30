//
// Created by Michael Ankele on 2025-04-30.
//

#include "MaterialPanel.h"
#include <Session.h>


string file_secure(const Path &filename);

MaterialPanel::MaterialPanel(ModeMaterial *_mode) : Node<xhui::Panel>("") {
	from_resource("material-panel");
	data = _mode->data;

	/*auto mat_list = (xhui::ListView*)get_control("materials");
	mat_list->column_factories[0].f_create = [this](const string& id) {
		return new XMaterialPanel(this, data, 0);
	};
	mat_list->column_factories[0].f_set = [this](xhui::Control* c, const string& t) {
		int i = t._int();
		reinterpret_cast<XMaterialPanel*>(c)->update(i);
	};
	mat_list->column_factories[0].f_select = [this](xhui::Control* c, bool selected) {
		reinterpret_cast<XMaterialPanel*>(c)->set_selected(selected);
	};

	data->out_material_changed >> create_sink([this] {
		if (apply_queue_depth == 0)
			load_data();
	});
	data->out_texture_changed >> create_sink([this] {
		if (apply_queue_depth == 0)
			load_data();
	});

	mode_mesh()->out_current_material_changed >> create_sink([this] { load_data(); });
	mode_mesh()->out_texture_level_changed >> create_sink([this] { load_data(); });

	popup_materials = xhui::create_resource_menu("model-material-list-popup");

	event_x("materials", xhui::event_id::Select, [this] { on_material_list_select(); });
	event_x("materials", xhui::event_id::RightButtonDown, [this] { on_material_list_right_click(); });
	event("add-new-material", [this] { on_material_add(); });
	event("load-material", [this] { on_material_load(); });
	event("delete-material", [this] { on_material_delete(); });
	event("apply-material", [this] { on_material_apply(); });



	set_visible("model_material_dialog_grp_color", full);
	set_visible("model_material_dialog_grp_transparency", full);*/

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
/*ModeMeshTexture *ModelMaterialPanel::mode_mesh_texture() {
	return mode_mesh()->mode_mesh_texture;
}*/

void MaterialPanel::fill_texture_list() {
	reset("textures");
	for (int i=0;i<data->appearance.texture_files.num;i++) {
		string id = format("image:material-texture[%d]", i);
	//	auto *img = mat->texture_levels[i]->image.get();
	//	auto *icon = mat->texture_levels[i]->image->scale(PREVIEW_SIZE, PREVIEW_SIZE);
	//	xhui::set_image(id, *icon);
		string ext = "";//format(" (%dx%d)", img->width, img->height);
		add_string("textures", format("Tex[%d]\\%s\\%s", i, id, (file_secure(data->appearance.texture_files[i]) + ext)));
	//	delete icon;
	}
	//	set_int("textures", mode_mesh()->current_texture_level);
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
