//
// Created by michi on 10/1/25.
//

#include "ModelRefPanel.h"
#include "../data/DataWorld.h"
#include <y/world/Model.h>
#include <ecs/Entity.h>
#include <storage/format/Format.h>
#include <storage/Storage.h>
#include <mode_material/dialog/MaterialSelector.h>


ModelRefPanel::ModelRefPanel(DataWorld* _data, int _index) : Node("model-panel") {
	from_source(R"foodelim(
Dialog model-panel ''
	Grid main-grid ''
		Grid ? ''
			Label ? 'Model' right disabled
			Button filename '' 'tooltip=Select model' expandx
			Button edit 'E' paddingx=5  'tooltip=Edit model' noexpandx primary
)foodelim");
	data = _data;
	index = _index;
	material_selector = new MaterialSelector(data);

	embed("main-grid", 0, 1, material_selector);

	auto e = data->entity(index);
	auto mr = e->get_component<ModelRef>();
	set_string("filename", str(mr->filename));
	if (mr->model) {
		material_selector->set_material(mr->model->material[0]);
		material_selector->internal_materials = mr->model->material;
	}

	event("filename", [this] {
		data->session->storage->file_dialog(FD_MODEL, false, true).then([this] (const ComplexPath& p) {
			auto e = data->entity(index);
			data->entity_edit_component(e, ModelRef::_class, {"", "", {{"filename", "", str(p.relative)}}});
			//material_selector->internal_materials = mr->model->material; ...
		});
	});
	event("edit", [this] {
		auto e = data->entity(index);
		auto m = e->get_component<ModelRef>();
		if (m->filename)
			data->session->universal_edit(FD_MODEL, m->filename, true);
	});

	material_selector->out_selected >> create_data_sink<yrenderer::Material*>([this, mr] (yrenderer::Material* m) {
		data->session->error("material changed... but not working");
		//mr->material...
		mr->model->material[0] = m->copy();
		data->out_changed();
		// FIXME :D
	});
}
