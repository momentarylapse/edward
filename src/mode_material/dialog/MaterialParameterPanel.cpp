//
// Created by michi on 3/19/26.
//

#include "MaterialParameterPanel.h"
#include <data/Data.h>
#include <Session.h>
#include <view/MaterialPreviewManager.h>
#include <storage/Storage.h>
#include <lib/image/image.h>
#include <lib/xhui/Menu.h>
#include <lib/xhui/Resource.h>
#include <lib/xhui/controls/ListView.h>
#include <lib/xhui/xhui.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/TextureManager.h>
#include <lib/yrenderer/Material.h>
#include <lib/yrenderer/MaterialManager.h>
#include <y/helper/ResourceManager.h>
#include <mode_material/dialog/MaterialSelectionDialog.h>
#include <mode_model/mesh/material/action/ActionModelEditMaterial.h>

string file_secure(const Path &filename);

ygfx::Texture* create_blank_texture() {
	constexpr int N = 512;
	auto t = new ygfx::Texture(N, N, "rgba:i8");
	t->write(Image(N, N, White));
	return t;
}

MaterialParameterPanel::MaterialParameterPanel(Data* _data, yrenderer::Material* _material) : Node("") {
	from_resource("material-parameter-panel");
	data = _data;
	session = data->session;
	material = _material;
	popup_textures = xhui::create_resource_menu("model-texture-list-popup");

	auto tex_list = (xhui::ListView*)get_control("textures");
	tex_list->column_factories[0].f_create = [](const string& id) {
		return xhui::create_control("Image", "!width=48,height=48", id);
	};
	tex_list->column_factories[1].f_create = [](const string& id) {
		return xhui::create_control("Label", "!markup", id);
	};

	event("texture-level-add", [this] { on_texture_level_add(); });
	event("textures", [this] { on_textures(); });
	event_x("textures", xhui::event_id::Select, [this] { on_textures_select(); });
	event_x("textures", xhui::event_id::RightButtonDown, [this] { on_textures_right_click(); });
	event("texture-level-delete", [this] { on_texture_level_delete(); });
	event("texture-level-clear", [this] { on_texture_level_clear(); });
	event("texture-level-load", [this] { on_texture_level_load(); });
	event("texture-level-save", [this] { on_texture_level_save(); });
	event("texture-level-scale", [this] { on_texture_level_scale(); });
	event("texture-level-linear", [this] { on_texture_level_linear(); });
	event("texture-level-srgb", [this] { on_texture_level_srgb(); });

	event("parent", [this] {
		MaterialSelectionDialog::ask(session, "Select parent material", {}, false, true).then([this] (yrenderer::Material* parent) {
			auto m = *material;
			m.derive_from(parent);
			data->execute(new ActionModelEditMaterial(material, m));
		});
	});

	event("reset-albedo", [this] {
		apply_queue_depth ++;
		auto m = *material;
		m.albedo = m.parent->albedo;
		data->execute(new ActionModelEditMaterial(material, m));
		update_ui();
		apply_queue_depth --;
	});
	event("reset-roughness", [this] {
		apply_queue_depth ++;
		auto m = *material;
		m.roughness = m.parent->roughness;
		data->execute(new ActionModelEditMaterial(material, m));
		update_ui();
		apply_queue_depth --;
	});
	event("reset-metal", [this] {
		apply_queue_depth ++;
		auto m = *material;
		m.metal = m.parent->metal;
		data->execute(new ActionModelEditMaterial(material, m));
		update_ui();
		apply_queue_depth --;
	});
	event("reset-emission", [this] {
		apply_queue_depth ++;
		auto m = *material;
		m.emission = m.parent->emission;
		data->execute(new ActionModelEditMaterial(material, m));
		update_ui();
		apply_queue_depth --;
	});
	event("albedo", [this] { apply_data_color(); });
	event("roughness", [this] { apply_data_color(); });
	event("slider-roughness", [this] { apply_data_color(); });
	event("metal", [this] { apply_data_color(); });
	event("slider-metal", [this] { apply_data_color(); });
	event("emission", [this] { apply_data_color(); });

	f_add_texture = [this] (shared<ygfx::Texture> t) {
		auto temp = *material;
		temp.textures.add(t);
		data->execute(new ActionModelEditMaterial(material, temp));
	};
	f_delete_texture = [this] (int index) {
		auto temp = *material;
		temp.textures.erase(index);
		data->execute(new ActionModelEditMaterial(material, temp));
	};

	session->resource_manager->material_manager->out_material_edited >> create_data_sink<yrenderer::Material*>([this] (yrenderer::Material* m) {
		if (m == material) {
			if (apply_queue_depth == 0) {
				update_ui();
			} else {
				set_string("header", material_name());
				update_reset_buttons();
			}
		}
	});
}

void MaterialParameterPanel::set_material(yrenderer::Material* m) {
	material = m;
	update_ui();
}

string MaterialParameterPanel::material_name() const {
	auto mm = session->resource_manager->material_manager;
	return mm->describe(material) + (mm->has_changes(material) ? " *" : "");
}

void MaterialParameterPanel::update_ui() {
	auto m = material;

	set_string("parent", m->parent ? str(session->resource_manager->material_manager->get_filename(m->parent)) : "none");
	set_string("parent-preview", session->material_preview_manager->get(m->parent));

	set_color("albedo", m->albedo);
	set_float("roughness", m->roughness);
	set_float("slider-roughness", m->roughness);
	set_float("metal", m->metal);
	set_float("slider-metal", m->metal);
	set_color("emission", m->emission);

	fill_texture_list();
	update_reset_buttons();
}

void MaterialParameterPanel::update_reset_buttons() {
	auto m = material;
	enable("reset-albedo", m->parent and (m->albedo != m->parent->albedo));
	enable("reset-roughness", m->parent and (m->roughness != m->parent->roughness));
	enable("reset-metal", m->parent and (m->metal != m->parent->metal));
	enable("reset-emission", m->parent and (m->emission != m->parent->emission));
}

// GUI -> data
void MaterialParameterPanel::apply_data_color() {
	apply_queue_depth ++;

	auto temp = *material;

	temp.albedo = get_color("albedo");
	temp.roughness = get_float("slider-roughness");
	temp.metal = get_float("slider-metal");
	temp.emission = get_color("emission");

	set_float("metal", temp.metal);
	set_float("roughness", temp.roughness);

	data->execute(new ActionModelEditMaterial(material, temp));
	this->apply_queue_depth --;
}

void MaterialParameterPanel::fill_texture_list() {
	auto mat = material;
	reset("textures");
	for (int i=0;i<mat->textures.num;i++) {
		auto t = mat->textures[i].get();
		string id = xhui::texture_to_image(t);
		string space = (t->color_space() == ColorSpace::Linear) ? "linear" : "srgb";
		string ext = format("\n<span size='small' alpha='50%%>   %d x %d, %s</span>", t->width, t->height, space);
	//	if (mat->texture_levels[i]->edited)
	//		ext += " *";
		add_string("textures", format("%s\\%s", id, (file_secure(session->resource_manager->texture_manager->texture_file(t)).replace("@linear", "") + ext)));
	}
//	set_int("textures", mode_mesh()->current_texture_level);
}


void MaterialParameterPanel::on_texture_level_add() {
	if (material->textures.num >= MATERIAL_MAX_TEXTURES) {
		session->error(format("Only %d texture levels allowed!", MATERIAL_MAX_TEXTURES));
		return;
	}

	f_add_texture(create_blank_texture());
}

void MaterialParameterPanel::on_textures() {
	on_textures_select();
	on_texture_level_load();
}

void MaterialParameterPanel::on_texture_level_load() {
	int sel = get_int("textures");
	if (sel >= 0) {
		session->storage->file_dialog(FD_TEXTURE, false, true).then([this, sel] (const auto& p) {
			auto temp = *material;
			auto rm = session->resource_manager;
			temp.textures[sel] = rm->load_texture(p.relative);
			data->execute(new ActionModelEditMaterial(material, temp));
		});
	}
}

void MaterialParameterPanel::on_texture_level_save() {
	int sel = get_int("textures");
	if (sel >= 0)
		session->storage->file_dialog(FD_TEXTURE, true, true).then([this, sel] (const auto& p) {
			auto t = material->textures[sel];
			Image im(t->width, t->height, Black);
			t->read(&im.data.data);
			im.save(p.complete);

			auto temp = *material;
			temp.textures[sel] = session->resource_manager->load_texture(p.relative);
			data->execute(new ActionModelEditMaterial(material, temp));
		});
}

void MaterialParameterPanel::on_texture_level_scale() {
	session->error("texture scale");
	/*int sel = get_int("textures");
	if (sel >= 0) {
		auto& tl = data->material[mode_mesh()->current_material]->texture_levels[sel];
		int w = tl.image->width;
		int h = tl.image->height;
		TextureScaleDialog::ask(win, w, h, [this, sel] (int _w, int _h) {
			data->execute(new ActionModelMaterialScaleTexture(mode_mesh()->current_material, sel, _w, _h));
		});
	}*/
}

void MaterialParameterPanel::on_textures_select() {
	int sel = get_int("textures");
//	mode_mesh()->set_current_texture_level(sel);
}

void MaterialParameterPanel::on_texture_level_delete() {
	int sel = get_int("textures");
	if (sel >= 0) {
		if (material->textures.num <= 1) {
			session->error("At least one texture level has to exist!");
			return;
		}
		f_delete_texture(sel);
	}
}

void MaterialParameterPanel::on_texture_level_clear() {
	int sel = get_int("textures");
	if (sel >= 0) {
		auto temp = *material;
		temp.textures[sel] = create_blank_texture();
		data->execute(new ActionModelEditMaterial(material, temp));
	}
}

void MaterialParameterPanel::on_texture_level_linear() {
	int sel = get_int("textures");
	if (sel >= 0) {
		auto temp = *material;
		auto rm = session->resource_manager;
		temp.textures[sel] = rm->load_texture(rm->texture_manager->texture_file(temp.textures[sel].get()).with("@linear"));
		data->execute(new ActionModelEditMaterial(material, temp));
	}
}

void MaterialParameterPanel::on_texture_level_srgb() {
	int sel = get_int("textures");
	if (sel >= 0) {
		auto temp = *material;
		auto rm = session->resource_manager;
		temp.textures[sel] = rm->load_texture(str(rm->texture_manager->texture_file(temp.textures[sel].get())).replace("@linear", ""));
		data->execute(new ActionModelEditMaterial(material, temp));
	}
}

void MaterialParameterPanel::on_textures_right_click() {
	int n = get_int("textures");
	if (n >= 0) {
		//mode_mesh()->set_current_texture_level(n);
	}
	popup_textures->enable("texture-level-delete", n>=0);
	popup_textures->enable("texture-level-clear", n>=0);
	popup_textures->enable("texture-level-load", n>=0);
	popup_textures->enable("texture-level-save", n>=0);
	popup_textures->enable("texture-level-scale", n>=0);
	popup_textures->enable("texture-level-linear", n>=0);
	popup_textures->enable("texture-level-srgb", n>=0);
	popup_textures->open_popup(this);
}

