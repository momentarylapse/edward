//
// Created by michi on 10/1/25.
//

#include "UserComponentPanel.h"

#include "../../data/DataWorld.h"
#include <ecs/Entity.h>
#include <storage/Storage.h>
#include <stuff/PluginManager.h>
#include <view/helper/InstanceEditor.h>
#include <lib/base/iter.h>
#include <lib/kaba/syntax/Class.h>


UserComponentPanel::UserComponentPanel(DataWorld* _data, int _index, const kaba::Class* _type) : ComponentContentsPanel(_data, _index) {
	from_source(R"foodelim(
Dialog user-component-panel ''
	Grid grid ''
)foodelim");
	type = _type;

	editor = new InstanceEditor(data->session, type, [this] (const plugin::InstanceData& desc) {
		if (auto e = data->entity(index)) {
			editing = true;
			data->entity_edit_component(e, type, desc);
			editing = false;
		}
	});

	set_string("group-component", type->name);
	embed("grid", 0, 0, editor.get());
	auto e = data->entity(index);
	if (auto c = e->_get_component_generic_(type))
		editor->build(c);
}

void UserComponentPanel::update_ui() {
	auto e = data->entity(index);
	if (auto c = e->_get_component_generic_(type))
		editor->update_ui(c);
}
