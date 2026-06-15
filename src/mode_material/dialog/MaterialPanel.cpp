//
// Created by Michael Ankele on 2025-04-30.
//

#include "MaterialPanel.h"
#include "MaterialPassPanel.h"
#include "MaterialParameterPanel.h"
#include "../action/ActionMaterialEditAppearance.h"
#include <Session.h>
#include <helper/ResourceManager.h>
#include <storage/Storage.h>
#include <lib/xhui/controls/ListView.h>
#include <storage/format/Format.h>
#include <view/DocumentSession.h>


string file_secure(const Path &filename);

MaterialPanel::MaterialPanel(ModeMaterial *_mode) : Node<xhui::Panel>("") {
	from_resource("material-panel");
	data = _mode->data;

	material_parameter_panel = new MaterialParameterPanel(_mode->session, data, &data->material);
	embed("parameter-grid", 0, 0, material_parameter_panel);


	auto pass_list = (xhui::ListView*)get_control("passes");
	pass_list->column_factories[0].f_create = [this](const string& id) {
		return new MaterialPassPanel(this, data, 0);
	};
	pass_list->column_factories[0].f_set = [](xhui::Control* c, const string& t) {
		int i = t._int();
		reinterpret_cast<MaterialPassPanel*>(c)->set_pass_no(i);
	};
	pass_list->column_factories[0].f_select = [](xhui::Control* c, bool selected) {
		reinterpret_cast<MaterialPassPanel*>(c)->set_selected(selected);
	};
	event("cast-shadows", [this] {
		on_edit();
	});
	event("add-pass", [this] {
		auto a = data->material;
		a.set_num_passes(a.num_passes + 1);
		data->execute(new ActionMaterialEditAppearance(a));
	});
	event("rcstatic", [this] {
		on_edit();
	});
	event("rcsliding", [this] {
		on_edit();
	});
	event("rcroll", [this] {
		on_edit();
	});
	event("rcjump", [this] {
		on_edit();
	});

	data->out_changed >> create_sink([this] {
		if (apply_queue_depth == 0) {
			load_data();
		}
	});

	material_parameter_panel->set_material(&data->material);
	load_data();
	apply_queue_depth = 0;
}

MaterialPanel::~MaterialPanel() {
	mode_material()->unsubscribe(this);
	data->unsubscribe(this);
}

void MaterialPanel::on_edit() {
	apply_queue_depth ++;
	auto a = data->material;
	a.cast_shadow = is_checked("cast-shadows");
	a.friction._static = get_float("rcstatic");
	a.friction.sliding = get_float("rcsliding");
	a.friction.rolling = get_float("rcroll");
	a.friction.jump = get_float("rcjump");
	data->execute(new ActionMaterialEditAppearance(a));
	apply_queue_depth --;
}

ModeMaterial* MaterialPanel::mode_material() {
	return data->doc->mode_material;
}

// data -> GUI
void MaterialPanel::load_data() {
	check("cast-shadows", data->material.cast_shadow);

	reset("passes");
	for (int i=0;i<data->material.num_passes;i++) {
		add_string("passes", str(i));
	}

	set_float("rcstatic", data->material.friction._static);
	set_float("rcsliding", data->material.friction.sliding);
	set_float("rcroll", data->material.friction.rolling);
	set_float("rcjump", data->material.friction.jump);
}

#if 0
void MaterialPanel::on_texture_level_add() {
	if (data->material.textures.num >= MATERIAL_MAX_TEXTURES) {
		data->session->error(format("Only %d texture levels allowed!", MATERIAL_MAX_TEXTURES));
		return;
	}

	auto a = data->material;
	a.textures.add(data->session->resource_manager->load_texture(""));
	data->execute(new ActionMaterialEditAppearance(a));
}

void MaterialPanel::on_texture_level_load() {
	int index = get_int("textures");
	if (index >= 0)
		data->session->storage->file_dialog(FD_TEXTURE, false, true).then([this, index] (const auto& p) {
			auto a = data->material;
			a.textures[index] = data->session->resource_manager->load_texture(p.relative);
			data->execute(new ActionMaterialEditAppearance(a));
		});
}

void MaterialPanel::on_texture_level_delete() {
	int index = get_int("textures");
	if (index >= 0) {
		if (data->material.textures.num <= 1) {
			data->session->error("At least one texture level has to exist!");
			return;
		}

		auto a = data->material;
		a.textures.erase(index);
		data->execute(new ActionMaterialEditAppearance(a));
	}
}

void MaterialPanel::on_texture_level_clear() {
	int index = get_int("textures");
	if (index >= 0) {
		auto a = data->material;
		a.textures[index] = data->session->resource_manager->load_texture("");
		data->execute(new ActionMaterialEditAppearance(a));
	}
}
#endif
