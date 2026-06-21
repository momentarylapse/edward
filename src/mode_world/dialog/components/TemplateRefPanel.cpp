//
// Created by michi on 6/22/26.
//

#include "TemplateRefPanel.h"
#include "../../data/DataWorld.h"
#include <y/world/Model.h>
#include <y/helper/ResourceManager.h>
#include <y/ecs/Entity.h>
#include <storage/format/Format.h>
#include <storage/Storage.h>
#include <mode_material/dialog/MaterialSelector.h>


TemplateRefPanel::TemplateRefPanel(DataWorld* _data, int _index) : ComponentContentsPanel(_data, _index) {
	from_source(R"foodelim(
Dialog template-ref-panel ''
	Grid main-grid ''
		Grid ? ''
			Label ? 'Template' right disabled
			Button template '' 'tooltip=Select template' ellipsis expandx
			Button edit 'E' paddingx=5  'tooltip=Edit template' noexpandx primary
)foodelim");

	auto e = data->entity(index);
	auto mr = e->get_component<ModelRef>();

	event("template", [this] {
		data->session->storage->file_dialog(FD_TEMPLATE, false, true).then([this] (const ComplexPath& p) {
			auto e = data->entity(index);
			data->entity_edit_component(e, TemplateRef::_class, {"", "", {{"template", str(p.relative)}}});
			//auto tr = e->get_component<TemplateRef>();
		});
	});
	event("edit", [this] {
		auto e = data->entity(index);
		auto tr = e->get_component<TemplateRef>();
		if (tr->_template)
			data->session->universal_edit(FD_TEMPLATE, data->session->resource_manager->filename(tr->_template), true);
	});
}

void TemplateRefPanel::update_ui() {
	auto e = data->entity(index);
	if (auto tr = e->get_component<TemplateRef>()) {
		set_string("template", str(data->session->resource_manager->filename(tr->_template)));
	}
}

