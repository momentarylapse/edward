//
// Created by michi on 10/1/25.
//

#include "ModelRefPanel.h"
#include "../../data/DataWorld.h"
#include <y/world/Model.h>
#include <ecs/Entity.h>
#include <storage/format/Format.h>
#include <storage/Storage.h>
#include <mode_material/dialog/MaterialSelector.h>

#include "helper/ResourceManager.h"
#include "world/ModelManager.h"


ModelRefPanel::ModelRefPanel(DataWorld* _data, int _index) : Node("model-panel") {
	from_source(R"foodelim(
Dialog model-panel ''
	Grid main-grid ''
		Grid ? ''
			Label ? 'Model' right disabled
			Button model '' 'tooltip=Select model' expandx
			Button edit 'E' paddingx=5  'tooltip=Edit model' noexpandx primary
)foodelim");
	data = _data;
	index = _index;
	material_selector = new MaterialSelector(data);

	embed("main-grid", 0, 1, material_selector);

	auto e = data->entity(index);
	auto mr = e->get_component<ModelRef>();

	event("model", [this] {
		data->session->storage->file_dialog(FD_MODEL, false, true).then([this] (const ComplexPath& p) {
			auto e = data->entity(index);
			data->entity_edit_component(e, ModelRef::_class, {"", "", {{"model", str(p.relative)}}});
			//material_selector->internal_materials = mr->model->material; ...
		});
	});
	event("edit", [this] {
		auto e = data->entity(index);
		auto mr = e->get_component<ModelRef>();
		if (mr->model)
			data->session->universal_edit(FD_MODEL, data->session->resource_manager->filename(mr->model), true);
	});

	material_selector->out_selected >> create_data_sink<yrenderer::Material*>([this, mr] (yrenderer::Material* m) {
		data->session->error("material changed... but not working");
		//mr->material...
		mr->set_material(0, m);
		data->out_changed();
		// FIXME :D
	});

	data->out_changed >> create_sink([this] {
		if (true) //!editing)
			update_ui();
	});
	update_ui();
}

void ModelRefPanel::update_ui() {
	auto e = data->entity(index);
	auto mr = e->get_component<ModelRef>();
	set_string("model", str(data->session->resource_manager->filename(mr->model)));
	if (mr->model) {
		material_selector->set_material(mr->get_material(0));
		material_selector->internal_materials = mr->model->materials;
	}
}
