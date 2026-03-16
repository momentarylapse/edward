//
// Created by Michael Ankele on 2025-04-16.
//

#include "ModelMaterialPanel.h"
#include "../../ModeMesh.h"
//#include "../ModeMeshTexture.h"
#include "../../../ModeModel.h"
#include "../../../data/DataModel.h"
#include "../../../data/ModelMesh.h"
#include <lib/mesh/Polygon.h>
#include "../action/ActionModelAddMaterial.h"
#include "../action/ActionModelDeleteMaterial.h"
#include "../action/ActionModelEditMaterial.h"
#include <Session.h>
#include <storage/Storage.h>
#include <lib/image/image.h>
#include <lib/os/msg.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/xhui/Dialog.h>
#include <lib/xhui/Menu.h>
#include <lib/xhui/Resource.h>
#include <lib/xhui/controls/Grid.h>
#include <lib/xhui/controls/ListView.h>
#include <y/EngineData.h>
#include <view/MultiView.h>
#include <view/DocumentSession.h>
#include <view/MaterialPreviewManager.h>

#include <y/helper/ResourceManager.h>
#include <lib/yrenderer/MaterialManager.h>
#include <lib/yrenderer/TextureManager.h>

static constexpr int PREVIEW_SIZE = 48;

string file_secure(const Path &filename) {
	if (filename)
		return str(filename);
	return "[no file]";
}

class XMaterialPanel : public xhui::Panel {
public:
	XMaterialPanel(ModelMaterialPanel* _parent, DataModel* _data, int _index) : xhui::Panel("") {
		from_resource("model-material-dialog");
		parent = _parent;
		data = _data;
		index = _index;
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

		event("override-colors", [this] { on_override_colors(); });
		event("albedo", [this] { apply_data_color(); });
		event("roughness", [this] { apply_data_color(); });
		event("slider-roughness", [this] { apply_data_color(); });
		event("metal", [this] { apply_data_color(); });
		event("slider-metal", [this] { apply_data_color(); });
		event("emission", [this] { apply_data_color(); });
	}

	yrenderer::Material* material() const {
		return data->materials[index];
	}

	void update(int _index) {
		index = _index;

		auto m = material();
		int nt = 0;

		set_string("preview", data->session->material_preview_manager->get(m));
		set_string("header", file_secure(data->session->resource_manager->material_manager->get_filename(m->parent)));
		set_string("subheader", format("%d polygons", nt));

		check("override-colors", true);
		/*enable("albedo", col.user);
		enable("roughness", col.user);
		enable("slider-roughness", col.user);
		enable("metal", col.user);
		enable("slider-metal", col.user);
		enable("emission", col.user);*/
		set_color("albedo", m->albedo);
		set_float("roughness", m->roughness);
		set_float("slider-roughness", m->roughness);
		set_float("metal", m->metal);
		set_float("slider-metal", m->metal);
		set_color("emission", m->emission);

		fill_texture_list();
	}
	void set_selected(bool selected) {
		expand("contents", selected);
	}

	// GUI -> data
	void apply_data_color() {
		parent->apply_queue_depth ++;

		auto m = *data->materials[index];

		bool user= is_checked("override-colors");

		if (user) {
			m.albedo = get_color("albedo");
			m.roughness = get_float("slider-roughness");
			m.metal = get_float("slider-metal");
			m.emission = get_color("emission");
		} else if (m.parent) {
			m.albedo = m.parent->albedo;
			m.roughness = m.parent->roughness;
			m.metal = m.parent->metal;
			m.emission = m.parent->emission;
		}
		set_float("metal", m.metal);
		set_float("roughness", m.roughness);
		enable("albedo", user);
		enable("roughness", user);
		enable("slider-roughness", user);
		enable("metal", user);
		enable("slider-metal", user);
		enable("emission", user);

		data->execute(new ActionModelEditMaterial(index, m));
		this->parent->apply_queue_depth --;
	}


	void on_override_colors() {
		apply_data_color();
	}

	void fill_texture_list() {
		auto mat = material();
		reset("textures");
		for (int i=0;i<mat->textures.num;i++) {
			auto t = mat->textures[i].get();
			string id = xhui::texture_to_image(t);
			string space = (t->color_space() == ColorSpace::Linear) ? "linear" : "srgb";
			string ext = format("\n<span size='small' alpha='50%%>   %d x %d, %s</span>", t->width, t->height, space);
		//	if (mat->texture_levels[i]->edited)
		//		ext += " *";
			add_string("textures", format("%s\\%s", id, (file_secure(data->session->resource_manager->texture_manager->texture_file(t)).replace("@linear", "") + ext)));
		}
	//	set_int("textures", mode_mesh()->current_texture_level);
	}


	void on_texture_level_add() {
		auto temp = material();
		if (temp->textures.num >= MATERIAL_MAX_TEXTURES) {
			data->session->error(format("Only %d texture levels allowed!", MATERIAL_MAX_TEXTURES));
			return;
		}


		data->execute(new ActionModelMaterialAddTexture(index));
	}

	void on_textures() {
		on_textures_select();
		on_texture_level_load();
	}

	void on_texture_level_load() {
#if 0
		int sel = get_int("textures");
		if (sel >= 0)
			data->session->storage->file_dialog(FD_TEXTURE, false, true).then([this, sel] (const auto& p) {
				data->execute(new ActionModelMaterialLoadTexture(index, sel, p.relative));
			});
#endif
		msg_error("TODO");
	}

	void on_texture_level_save() {
#if 0
		int sel = get_int("textures");
		if (sel >= 0)
			data->session->storage->file_dialog(FD_TEXTURE, true, true).then([this, sel] (const auto& p) {
				auto tl = material()->texture_levels[sel];
				tl->image->save(p.complete);
				tl->filename = p.relative; // ...
				tl->edited = false;
			});
#endif
		msg_error("TODO");
	}

	void on_texture_level_scale() {
		msg_todo("texture scale");
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

	void on_textures_select() {
		int sel = get_int("textures");
	//	mode_mesh()->set_current_texture_level(sel);
	}

	void on_texture_level_delete() {
		int sel = get_int("textures");
		if (sel >= 0) {
			if (data->materials[index]->textures.num <= 1) {
				data->session->error("At least one texture level has to exist!");
				return;
			}
			data->execute(new ActionModelMaterialDeleteTexture(index, sel));
		}
	}

	void on_texture_level_clear() {
		msg_error("TODO");
#if 0
		int sel = get_int("textures");
		if (sel >= 0)
			data->execute(new ActionModelMaterialLoadTexture(index, sel, ""));
#endif
	}

	void on_texture_level_linear() {
		msg_error("TODO");
#if 0
		auto mat = material();
		int sel = get_int("textures");
		if (sel >= 0)
			data->execute(new ActionModelMaterialLoadTexture(index, sel, mat->texture_levels[sel]->filename.with("@linear")));
#endif
	}

	void on_texture_level_srgb() {
		msg_error("TODO");
#if 0
		auto mat = material();
		int sel = get_int("textures");
		if (sel >= 0)
			data->execute(new ActionModelMaterialLoadTexture(index, sel, str(mat->texture_levels[sel]->filename).replace("@linear", "")));
#endif
	}

	void on_textures_right_click() {
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

	ModelMaterialPanel* parent;
	DataModel* data;
	int index;
	owned<xhui::Menu> popup_textures;
};

ModelMaterialPanel::ModelMaterialPanel(DataModel *_data, bool full) : Node<xhui::Panel>("") {
	from_resource("model-material-panel");
	data = _data;

	auto mat_list = (xhui::ListView*)get_control("materials");
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

	mode_mesh()->out_current_material_changed >> create_sink([this] {
		set_int("materials", mode_mesh()->current_material);
	});
	mode_mesh()->out_texture_level_changed >> create_sink([this] { load_data(); });

	popup_materials = xhui::create_resource_menu("model-material-list-popup");

	event_x("materials", xhui::event_id::Select, [this] {
		on_material_list_select();
	});
	event_x("materials", xhui::event_id::RightButtonDown, [this] { on_material_list_right_click(); });
	event("add-new-material", [this] { on_material_add(); });
	event("load-material", [this] { on_material_load(); });
	event("delete-material", [this] { on_material_delete(); });
	event("apply-material", [this] { on_material_apply(); });



	//set_visible("model_material_dialog_grp_color", full);
	//set_visible("model_material_dialog_grp_transparency", full);

	load_data();
	apply_queue_depth = 0;
}

ModelMaterialPanel::~ModelMaterialPanel() {
	mode_mesh()->unsubscribe(this);
	data->unsubscribe(this);

	delete popup_materials;
}


ModeMesh *ModelMaterialPanel::mode_mesh() {
	return data->doc->mode_model->mode_mesh.get();
}
/*ModeMeshTexture *ModelMaterialPanel::mode_mesh_texture() {
	return mode_mesh()->mode_mesh_texture;
}*/

// data -> GUI
void ModelMaterialPanel::load_data() {
	fill_material_list();
}

int count_material_polygons(DataModel *data, int index) {
	int n = 0;
	for (auto &t: data->mesh->polygons)
		if (t.material == index)
			n ++;
	return n;
}

void ModelMaterialPanel::fill_material_list() {
	reset("materials");
	for (int i=0;i<data->materials.num;i++) {
		int nt = count_material_polygons(data, i);
		string im = data->session->material_preview_manager->get(data->materials[i]);
		add_string("materials", str(i)); //format("Mat[%d]\\%d\\%s\\%s", i, nt, im, file_secure(data->material[i]->filename)));
		//add_string("material_list", format("Mat[%d]\\%d\\%s\\%s", i, nt, im, file_secure(data->material[i]->filename)));
	}
	set_int("materials", mode_mesh()->current_material);
}



void ModelMaterialPanel::on_material_list_select() {
	mode_mesh()->set_current_material(get_int("materials"));
}

void ModelMaterialPanel::on_material_add() {
	data->execute(new ActionModelAddMaterial(new yrenderer::Material));
}

void ModelMaterialPanel::on_material_load() {
	data->session->storage->file_dialog(FD_MATERIAL, false, true).then([this] (const auto& p) {
		auto m = new yrenderer::Material;
		m->derive_from(data->session->resource_manager->load_material(p.simple));
		data->execute(new ActionModelAddMaterial(m));
	});
}

void ModelMaterialPanel::on_material_delete() {
	if (count_material_polygons(data, mode_mesh()->current_material) > 0) {
		data->session->error("can only delete materials that are not applied to any polygons");
		return;
	}
	data->execute(new ActionModelDeleteMaterial(mode_mesh()->current_material));
}

void ModelMaterialPanel::on_material_apply() {
	data->apply_material(mode_mesh()->multi_view->selection, mode_mesh()->current_material);
}


class TextureScaleDialog : public xhui::Dialog {
public:
	TextureScaleDialog(xhui::Window *parent, int w, int h) : xhui::Dialog("Scale texture", 300, 100, parent, xhui::DialogFlags::None) {
		width = height = -1;
		from_resource("texture-scale-dialog");
		set_int("width", w);
		set_int("height", h);
		event("ok", [this] { on_ok(); });
		event("cancel", [this] { on_cancel(); });
	}
	void on_ok() {
		width = get_int("width");
		height = get_int("height");
		request_destroy();
	}
	void on_cancel() {
		request_destroy();
	}
	int width, height;

	static void ask(xhui::Window *parent, int &w, int &h, std::function<void(int,int)> cb_success) {
		auto *dlg = new TextureScaleDialog(parent, w, h);
		parent->open_dialog(dlg).then([dlg, cb_success] {
			if (dlg->width > 0)
				cb_success(dlg->width, dlg->height);
		});
	}
};

void ModelMaterialPanel::on_material_list_right_click() {
	int n = get_int("materials");
	if (n >= 0) {
		mode_mesh()->set_current_material(n);
		popup_materials->enable("apply-material", n>=0);
		popup_materials->enable("delete-material", n>=0);
		popup_materials->open_popup(this);
	}
}
