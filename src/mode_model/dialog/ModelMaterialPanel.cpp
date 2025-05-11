//
// Created by Michael Ankele on 2025-04-16.
//

#include "ModelMaterialPanel.h"
#include "../mesh/ModeMesh.h"
//#include "../mesh/ModeMeshTexture.h"
#include "../ModeModel.h"
#include "../data/DataModel.h"
#include "../data/ModelMesh.h"
#include "../../data/mesh/Polygon.h"
#include "../data/ModelMaterial.h"
#include "../action/data/ActionModelAddMaterial.h"
#include "../action/data/ActionModelDeleteMaterial.h"
#include "../action/data/ActionModelEditMaterial.h"
#include "../action/data/ActionModelEditData.h"
#include "../../Session.h"
#include "../../storage/Storage.h"
#include <lib/image/image.h>
#include <lib/os/msg.h>
#include <lib/xhui/Dialog.h>
#include <lib/xhui/Menu.h>
#include <lib/xhui/Resource.h>
#include <lib/xhui/controls/Grid.h>
#include <lib/xhui/controls/Image.h>
#include <lib/xhui/controls/ListView.h>
#include <y/EngineData.h>

static constexpr int PREVIEW_SIZE = 48;

string file_secure(const Path &filename) {
	if (filename)
		return str(filename);
	return "<no file>";
}

string render_material(ModelMaterial *m) {
	string uid = "image:" + p2s(m);
	Image im(PREVIEW_SIZE, PREVIEW_SIZE, White);
	const vec3 L = vec3(-0.2f, 0.6f, 1).normalized();
	for (int i=0; i<PREVIEW_SIZE; i++)
		for (int j=0; j<PREVIEW_SIZE; j++) {
			color c = m->col.albedo;
			if (m->texture_levels.num >= 1) {
				auto t = m->texture_levels[0]->image.get();
				c *= t->get_pixel(i * t->width / PREVIEW_SIZE, j * t->height / PREVIEW_SIZE);
			}
			const vec2 p0 = vec2((float)i / PREVIEW_SIZE - 0.5f, (float)j / PREVIEW_SIZE - 0.5f) * 2;
			float r = p0.length();
			if (r < 1) {
				vec3 n = vec3(p0.x, p0.y, -sqrt(1 - r*r));
				float d = clamp(-vec3::dot(n, L), 0.0f, 1.0f);
				c = c * (d*(1-m->col.roughness) + m->col.roughness);
				c += m->col.emission;
			} else {
				c = color(0,0,0,0);
			}
			c.clamp();

			im.set_pixel(i, j, c);
		}
	// TODO really render!
	xhui::set_image(uid, im);
	return uid;
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
		tex_list->column_factories[1].f_create = [](const string& id) {
			return new xhui::Image(id, "");
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

		event("override-colors", [this] { on_override_colors(); });
		event("albedo", [this] { apply_data_color(); });
		event("roughness", [this] { apply_data_color(); });
		event("slider-roughness", [this] { apply_data_color(); });
		event("metal", [this] { apply_data_color(); });
		event("slider-metal", [this] { apply_data_color(); });
		event("emission", [this] { apply_data_color(); });
	}

	ModelMaterial* material() const {
		return data->material[index];
	}

	void update(int _index) {
		index = _index;

		auto m = material();
		int nt = 0;

		set_string("preview", render_material(m));
		set_string("header", file_secure(m->filename));
		set_string("subheader", format("%d polygons", nt));

		auto& col = m->col;
		check("override-colors", col.user);
		enable("albedo", col.user);
		enable("roughness", col.user);
		enable("slider-roughness", col.user);
		enable("metal", col.user);
		enable("slider-metal", col.user);
		enable("emission", col.user);
		set_color("albedo", col.albedo);
		set_float("roughness", col.roughness);
		set_float("slider-roughness", col.roughness);
		set_float("metal", col.metal);
		set_float("slider-metal", col.metal);
		set_color("emission", col.emission);

		fill_texture_list();
	}
	void set_selected(bool selected) {
		expand("grp-color", selected);
		expand("grp-textures", selected);
		set_visible("grp-color", selected);
		set_visible("grp-textures", selected);
	}

	// GUI -> data
	void apply_data_color() {
		parent->apply_queue_depth ++;

		auto m = data->material[index];
		auto col = m->col;
		auto parent = m->material.get();

		col.user= is_checked("override-colors");

		if (col.user) {
			col.albedo = get_color("albedo");
			col.roughness = get_float("slider-roughness");
			col.metal = get_float("slider-metal");
			col.emission = get_color("emission");
		} else {
			col.albedo = parent->albedo;
			col.roughness = parent->roughness;
			col.metal = parent->metal;
			col.emission = parent->emission;
		}
		set_float("metal", col.metal);
		set_float("roughness", col.roughness);
		enable("albedo", col.user);
		enable("roughness", col.user);
		enable("slider-roughness", col.user);
		enable("metal", col.user);
		enable("slider-metal", col.user);
		enable("emission", col.user);

		data->execute(new ActionModelEditMaterial(index, col));
		this->parent->apply_queue_depth --;

		set_string("preview", render_material(m));
	}


	void on_override_colors() {
		apply_data_color();
	}

	void fill_texture_list() {
		auto mat = material();
		reset("textures");
		for (int i=0;i<mat->texture_levels.num;i++) {
			string id = format("image:material[%d]-texture[%d]", index, i);
			auto *img = mat->texture_levels[i]->image.get();
			auto *icon = img->scale(PREVIEW_SIZE, PREVIEW_SIZE);
			xhui::set_image(id, *icon);
			string ext = format(" (%dx%d)", img->width, img->height);
			if (mat->texture_levels[i]->edited)
				ext += " *";
			add_string("textures", format("Tex[%d]\\%s\\%s", i, id, (file_secure(mat->texture_levels[i]->filename) + ext)));
			delete icon;
		}
	//	set_int("textures", mode_mesh()->current_texture_level);
	}


	void on_texture_level_add() {
		auto temp = material();
		if (temp->texture_levels.num >= MATERIAL_MAX_TEXTURES) {
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
		int sel = get_int("textures");
		if (sel >= 0)
			data->session->storage->file_dialog(FD_TEXTURE, false, true).then([this, sel] (const auto& p) {
				data->execute(new ActionModelMaterialLoadTexture(index, sel, p.relative));
			});
	}

	void on_texture_level_save() {
		int sel = get_int("textures");
		if (sel >= 0)
			data->session->storage->file_dialog(FD_TEXTURE, true, true).then([this, sel] (const auto& p) {
				auto tl = material()->texture_levels[sel];
				tl->image->save(p.complete);
				tl->filename = p.relative; // ...
				tl->edited = false;
			});
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
			if (data->material[index]->texture_levels.num <= 1) {
				data->session->error("At least one texture level has to exist!");
				return;
			}
			data->execute(new ActionModelMaterialDeleteTexture(index, sel));
		}
	}

	void on_texture_level_clear() {
		int sel = get_int("textures");
		if (sel >= 0)
			data->execute(new ActionModelMaterialLoadTexture(index, sel, ""));
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
	set_visible("model_material_dialog_grp_transparency", full);

	load_data();
	apply_queue_depth = 0;
}

ModelMaterialPanel::~ModelMaterialPanel() {
	mode_mesh()->unsubscribe(this);
	data->unsubscribe(this);

	delete popup_materials;
}


ModeMesh *ModelMaterialPanel::mode_mesh() {
	return data->session->mode_model->mode_mesh.get();
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
	for (int i=0;i<data->material.num;i++) {
		int nt = count_material_polygons(data, i);
		string im = render_material(data->material[i]);
		add_string("materials", str(i)); //format("Mat[%d]\\%d\\%s\\%s", i, nt, im, file_secure(data->material[i]->filename)));
		//add_string("material_list", format("Mat[%d]\\%d\\%s\\%s", i, nt, im, file_secure(data->material[i]->filename)));
	}
	set_int("materials", mode_mesh()->current_material);
}



void ModelMaterialPanel::on_material_list_select() {
	mode_mesh()->set_current_material(get_int("materials"));
}

void ModelMaterialPanel::on_material_add() {
	data->execute(new ActionModelAddMaterial(""));
}

void ModelMaterialPanel::on_material_load() {
	data->session->storage->file_dialog(FD_MATERIAL, false, true).then([this] (const auto& p){
		data->execute(new ActionModelAddMaterial(p.simple));
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
	data->apply_material(data->get_selection(), mode_mesh()->current_material);
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
