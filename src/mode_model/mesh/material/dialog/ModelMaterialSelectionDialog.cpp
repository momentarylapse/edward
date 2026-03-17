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

#include "lib/os/filesystem.h"
#include "lib/os/msg.h"


string file_secure(const Path &filename);

ModelMaterialSelectionDialog::ModelMaterialSelectionDialog(Session* _session, const Array<yrenderer::Material*>& _internal_materials) : Dialog("", _session->win) {
	session = _session;
	materials = _internal_materials;
	from_resource("model_material_selection_dialog");

	auto list = (xhui::ListView*)get_control("material_list");
	list->column_factories[0].f_create = [](const string& id) {
		return xhui::create_control("Image", "", id);
	};
	list->column_factories[1].f_create = [](const string& id) {
		return xhui::create_control("Label", "!markup", id);
	};

	event("apply", [this] {
		int i = get_int("material_list");
		promise(materials[i]);
		request_destroy();
	});
	event("material_list", [this] {
		int i = get_int("material_list");
		promise(materials[i]);
		request_destroy();
	});
	event_x(id, xhui::event_id::Close, [this] {
		promise.fail();
		request_destroy();
	});


	auto mm = session->resource_manager->material_manager;


	for (const auto &f: os::fs::search(mm->material_dir, "*.material", "-fr")) {
		msg_write(str(f));
		auto m = mm->load(f.no_ext());
		materials.add(m);
	}

	for (auto m: materials) {
		//int nt = count_material_polygons(data, i);
		string im = session->material_preview_manager->get(m);
		string name = str(mm->get_filename(m));
		if (name == "")
			name = "[internal]";
		if (m->parent)
			name += format("\n <span size='small' alpha='50%%>derived from %s</span>", mm->get_filename(m->parent));
		add_string("material_list", format("%s\\%s", im, name));
	}
	//set_int("materials", mode_mesh->current_material);
}


base::future<yrenderer::Material*> ModelMaterialSelectionDialog::ask(Session* session, const Array<yrenderer::Material*>& internal_materials) {
	auto dlg = new ModelMaterialSelectionDialog(session, internal_materials);
	session->win->open_dialog(dlg);

	return dlg->promise.get_future();
}


