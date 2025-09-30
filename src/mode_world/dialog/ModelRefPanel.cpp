//
// Created by michi on 10/1/25.
//

#include "ModelRefPanel.h"
#include "../data/DataWorld.h"
#include <y/world/Model.h>
#include <y/y/Entity.h>
#include <storage/format/Format.h>

#include "storage/Storage.h"


ModelRefPanel::ModelRefPanel(DataWorld* _data, int _index) : Panel("model-panel") {
	from_source(R"foodelim(
Dialog model-panel ''
	Grid ? ''
		Label ? 'Model'
		Button filename '' expandx
		Button edit 'E' noexpandx primary
)foodelim");
	data = _data;
	index = _index;
	auto e = data->entity(index);
	auto m = e->get_component<ModelRef>();
	set_string("filename", str(m->filename));
	event("filename", [this] {
		data->session->storage->file_dialog(FD_MODEL, false, true).then([this] (const ComplexPath& p) {
			auto e = data->entity(index);
			auto m = e->get_component<ModelRef>();
			// TODO action
			m->filename = p.simple;
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
