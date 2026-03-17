//
// Created by Michael Ankele on 2025-04-18.
//

#include "ModelMaterialSelectionDialog.h"
#include <Session.h>
#include <lib/xhui/controls/Image.h>
#include <lib/xhui/controls/ListView.h>
#include <view/EdwardWindow.h>
#include <view/MaterialPreviewManager.h>
#include <mode_model/mesh/ModeMesh.h>

#include "helper/ResourceManager.h"
#include "lib/yrenderer/MaterialManager.h"


string file_secure(const Path &filename);

ModelMaterialSelectionDialog::ModelMaterialSelectionDialog(ModeMesh* _mode_mesh) : Dialog("", _mode_mesh->session->win) {
	mode_mesh = _mode_mesh;
	from_resource("model_material_selection_dialog");

	auto list = (xhui::ListView*)get_control("material_list");
	list->column_factories[0].f_create = [](const string& id) {
		return new xhui::Image(id, "");
	};
	list->column_factories[1].f_create = [](const string& id) {
		return new xhui::Label(id, "!markup");
	};

	event("apply", [this] {
		promise(get_int("material_list"));
		request_destroy();
	});
	event("material_list", [this] {
		promise(get_int("material_list"));
		request_destroy();
	});
	event_x(id, xhui::event_id::Close, [this] {
		promise.fail();
		request_destroy();
	});


	auto mm = mode_mesh->session->resource_manager->material_manager;
	for (auto m: mode_mesh->data->materials) {
		//int nt = count_material_polygons(data, i);
		string im = mode_mesh->session->material_preview_manager->get(m);
		string name = str(mm->get_filename(m));
		if (name == "")
			name = "[internal]";
		if (m->parent)
			name += format("\n <span size='small' alpha='50%%>derived from %s</span>", mm->get_filename(m->parent));
		add_string("material_list", format("%s\\%s", im, name));
	}
	//set_int("materials", mode_mesh->current_material);
}


base::future<int> ModelMaterialSelectionDialog::ask(ModeMesh* mode_mesh) {
	auto dlg = new ModelMaterialSelectionDialog(mode_mesh);
	mode_mesh->session->win->open_dialog(dlg);

	return dlg->promise.get_future();
}


