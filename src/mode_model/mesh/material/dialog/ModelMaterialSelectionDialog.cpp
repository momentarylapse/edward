//
// Created by Michael Ankele on 2025-04-18.
//

#include "ModelMaterialSelectionDialog.h"
#include <Session.h>
#include <lib/xhui/controls/Image.h>
#include <lib/xhui/controls/ListView.h>
#include <view/EdwardWindow.h>
#include <view/MaterialPreviewManager.h>
#include <y/helper/ResourceManager.h>
#include <lib/yrenderer/MaterialManager.h>
#include <lib/os/filesystem.h>


string file_secure(const Path &filename);

yrenderer::Material* ModelMaterialSelectionDialog::new_material = nullptr;

ModelMaterialSelectionDialog::ModelMaterialSelectionDialog(Session* _session, const string& title, const Array<yrenderer::Material*>& _internal_materials, bool allow_new, bool allow_none) : Dialog("", _session->win) {
	session = _session;
	if (!new_material)
		new_material = session->resource_manager->material_manager->create_internal();

	if (allow_none)
		materials.add(nullptr);
	materials.append(_internal_materials);
	if (allow_new) {
		materials.add(new_material);
	}
	from_resource("model_material_selection_dialog");
	set_title(title);

	auto list = (xhui::ListView*)get_control("material_list");
	list->column_factories[0].f_create = [](const string& id) {
		return xhui::create_control("Image", "!width=48,height=48", id);
	};
	list->column_factories[1].f_create = [](const string& id) {
		return xhui::create_control("Label", "!markup", id);
	};

	auto select_and_close = [this] {
		int i = get_int("material_list");
		promise(materials[i] == new_material ? session->resource_manager->material_manager->create_internal() : materials[i]);
		request_destroy();
	};

	event("apply", select_and_close);
	event("material_list", select_and_close);
	event_x(id, xhui::event_id::Close, [this] {
		promise.fail();
		request_destroy();
	});


	auto mm = session->resource_manager->material_manager;


	for (const auto &f: os::fs::search(mm->material_dir, "*.material", "-fr")) {
		auto m = mm->load(f.no_ext());
		materials.add(m);
	}

	for (auto m: materials) {
		//int nt = count_material_polygons(data, i);
		string im = session->material_preview_manager->get(m);
		string name = mm->describe(m);
		if (m == new_material)
			name = "[create new]";
		add_string("material_list", format("%s\\%s", im, name));
	}
	//set_int("materials", mode_mesh->current_material);
}


base::future<yrenderer::Material*> ModelMaterialSelectionDialog::ask(Session* session, const string& title, const Array<yrenderer::Material*>& internal_materials, bool allow_new, bool allow_none) {
	auto dlg = new ModelMaterialSelectionDialog(session, title, internal_materials, allow_new, allow_none);
	session->win->open_dialog(dlg);

	return dlg->promise.get_future();
}


