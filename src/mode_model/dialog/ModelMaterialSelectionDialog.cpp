//
// Created by Michael Ankele on 2025-04-18.
//

#include "ModelMaterialSelectionDialog.h"
#include <Session.h>
#include <lib/xhui/controls/Image.h>
#include <lib/xhui/controls/ListView.h>
#include <view/EdwardWindow.h>
#include <mode_model/mesh/ModeMesh.h>


string file_secure(const Path &filename);
string render_material(ModelMaterial *m);

ModelMaterialSelectionDialog::ModelMaterialSelectionDialog(ModeMesh* _mode_mesh) : Dialog("", _mode_mesh->session->win) {
	mode_mesh = _mode_mesh;
	from_resource("model_material_selection_dialog");

	auto list = (xhui::ListView*)get_control("material_list");
	list->column_factories[0].f_create = [](const string& id) {
		return new xhui::Image(id, "");
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


	for (int i=0;i<mode_mesh->data->material.num;i++) {
		//int nt = count_material_polygons(data, i);
		string im = render_material(mode_mesh->data->material[i]);
		add_string("material_list", format("%s\\%s", im, file_secure(mode_mesh->data->material[i]->filename)));
		//add_string("material_list", format("Mat[%d]\\%d\\%s\\%s", i, nt, im, file_secure(data->material[i]->filename)));
	}
	//set_int("materials", mode_mesh->current_material);
}


base::future<int> ModelMaterialSelectionDialog::ask(ModeMesh* mode_mesh) {
	auto dlg = new ModelMaterialSelectionDialog(mode_mesh);
	mode_mesh->session->win->open_dialog(dlg);

	return dlg->promise.get_future();
}


