//
// Created by Michael Ankele on 2025-04-16.
//

#include "ModelMaterialPanel.h"
#include <mode_material/dialog/MaterialParameterPanel.h>
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

#include "ModelMaterialSelectionDialog.h"
#include "lib/base/iter.h"
#include "mode_model/mesh/ModeMesh.h"
#include "world/ModelManager.h"

static constexpr int PREVIEW_SIZE = 48;

string file_secure(const Path &filename) {
	if (filename)
		return str(filename);
	return "[no file]";
}

int count_material_polygons(DataModel *data, int index) {
	int n = 0;
	for (auto &t: data->mesh->polygons)
		if (t.material == index)
			n ++;
	return n;
}


class InlineMaterialEditorPanel : public obs::Node<xhui::Panel> {
public:
	InlineMaterialEditorPanel(ModelMaterialPanel* _parent, DataModel* _data, int _index) : Node("") {
		from_resource("model-material-dialog");
		parent = _parent;
		data = _data;
		index = _index;

		material_parameter_panel = new MaterialParameterPanel(data, material());
		embed("contents-grid", 0, 0, material_parameter_panel);

		event("delete", [this] { on_delete(); });
		event("apply-to-selection", [this] { on_apply(); });
		event("select-polygons", [this] {
			auto mm = parent->mode_mesh();
			mm->set_presentation_mode(ModeMesh::PresentationMode::Polygons);
			mm->multi_view->clear_selection();
			for (const auto& [i, p]: enumerate(mm->data->mesh->polygons))
				if (p.material == index)
					mm->multi_view->selection[MultiViewType::MODEL_POLYGON].add(i);
			mm->multi_view->update_selection_box();
		});
		event("save", [this] {
		});
		event("save-as", [this] {
		});
		event("menu", [this] {
			auto menu = xhui::create_resource_menu("model-material-list-popup");
			xhui::open_popup_menu(get_control("menu"), menu);
		});

		data->session->resource_manager->material_manager->out_material_edited >> create_data_sink<yrenderer::Material*>([this] (yrenderer::Material* m) {
			if (m == material()) {
				if (parent->apply_queue_depth == 0) {
					update_ui();
				} else {
					set_string("header", material_name());
				}
			}
		});
	}

	void on_delete() {
		if (count_material_polygons(data, index) > 0) {
			data->session->error("can only delete materials that are not applied to any polygons");
			return;
		}
		data->execute(new ActionModelDeleteMaterial(index));
	}

	void on_apply() {
		data->apply_material(parent->mode_mesh()->multi_view->selection, index);
	}

	yrenderer::Material* material() const {
		return data->materials[index];
	}

	string material_name() const {
		auto mm = data->session->resource_manager->material_manager;
		return mm->describe(material()) + (mm->has_changes(material()) ? " *" : "");
	}

	void set_index(int _index) {
		index = _index;
		material_parameter_panel->set_material(material());
		update_ui();
	}

	void update_ui() {
		auto m = material();
		int nt = count_material_polygons(data, index);
		set_string("preview", data->session->material_preview_manager->get(m));
		set_string("header", material_name());
		set_string("subheader", format("%d polygons", nt));
	}

	void set_selected(bool selected) {
		expand("contents", selected);
	}

	ModelMaterialPanel* parent;
	DataModel* data;
	int index;
	MaterialParameterPanel* material_parameter_panel;
};

ModelMaterialPanel::ModelMaterialPanel(DataModel *_data, bool full) : Node<xhui::Panel>("") {
	from_resource("model-material-panel");
	data = _data;

	auto mat_list = (xhui::ListView*)get_control("materials");
	mat_list->column_factories[0].f_create = [this](const string& id) {
		return new InlineMaterialEditorPanel(this, data, 0);
	};
	mat_list->column_factories[0].f_set = [](xhui::Control* c, const string& t) {
		int i = t._int();
		reinterpret_cast<InlineMaterialEditorPanel*>(c)->set_index(i);
	};
	mat_list->column_factories[0].f_select = [](xhui::Control* c, bool selected) {
		reinterpret_cast<InlineMaterialEditorPanel*>(c)->set_selected(selected);
	};

	data->out_material_added_or_deleted >> create_sink([this] {
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

	event_x("materials", xhui::event_id::Select, [this] {
		on_material_list_select();
	});
	event("add", [this] {
		on_add();
	});


	load_data();
	apply_queue_depth = 0;
}

ModelMaterialPanel::~ModelMaterialPanel() {
	mode_mesh()->unsubscribe(this);
	data->unsubscribe(this);
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

void ModelMaterialPanel::on_add() {
	ModelMaterialSelectionDialog::ask(data->session, "Add a material", {}, true, false).then([this] (yrenderer::Material* material) {
		data->execute(new ActionModelAddMaterial(material));
		data->session->info("added material");
	});
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
