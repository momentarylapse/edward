//
// Created by michi on 10/1/25.
//

#include "ModelRefPanel.h"
#include "../data/DataWorld.h"
#include <y/world/Model.h>
#include <ecs/Entity.h>
#include <storage/format/Format.h>
#include <storage/Storage.h>
#include <view/dialogs/MaterialEditDialog.h>


ModelRefPanel::ModelRefPanel(DataWorld* _data, int _index) : Panel("model-panel") {
	from_source(R"foodelim(
Dialog model-panel ''
	Grid main-grid ''
		Grid ? ''
			Grid ? ''
				Label ? 'Model' right disabled
				Button filename '' 'tooltip=Select model' expandx
			---|
			Grid ? ''
				Label ? '' expandx
				Button edit 'Edit' 'tooltip=Edit model' noexpandx primary
)foodelim");
	data = _data;
	index = _index;

	material_edit_panel = new MaterialEditPanel(data);
	embed("main-grid", 0, 1, material_edit_panel);

	auto e = data->entity(index);
	auto m = e->get_component<ModelRef>();
	set_string("filename", str(m->filename));
	if (m->model)
		material_edit_panel->set_material(m->model->material[0]);

	event("filename", [this] {
		data->session->storage->file_dialog(FD_MODEL, false, true).then([this] (const ComplexPath& p) {
			auto e = data->entity(index);
			auto m = e->get_component<ModelRef>();
			// TODO action
			m->filename = p.relative;
			set_string("filename", str(m->filename));
			data->out_changed();
		});
	});
	event("edit", [this] {
		auto e = data->entity(index);
		auto m = e->get_component<ModelRef>();
		if (m->filename)
			data->session->universal_edit(FD_MODEL, m->filename, true);
	});
}
