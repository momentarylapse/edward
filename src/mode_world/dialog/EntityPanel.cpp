//
// Created by michi on 02.02.25.
//

#include "EntityPanel.h"
#include "EntityListPanel.h"
#include "AddEntityPanel.h"
#include "ComponentPanel.h"
#include "../ModeWorld.h"
#include "ComponentSelectionDialog.h"
#include <stuff/PluginManager.h>
#include <lib/xhui/controls/ListView.h>
#include <lib/xhui/dialogs/FileSelectionDialog.h>
#include <lib/xhui/xhui.h>
#include <view/MultiView.h>
#include <view/DocumentSession.h>
#include <ecs/Entity.h>
#include <y/world/components/Camera.h>
#include <storage/Storage.h>
#include <lib/base/iter.h>

#include "helper/ResourceManager.h"


EntityPanel::EntityPanel(ModeWorld* _mode) : obs::Node<xhui::Panel>("entity-panel") {
	mode_world = _mode;
	from_source(R"foodelim(
Dialog entity-panel '' padding=0
	Grid main-grid '' margin=7
		ListView components 'c' nobar sunkenbackground=no showselection=no selectsingle
		---|
		Grid ? ""
			Button add-component '+' 'tooltip=Select a component class to add to this entity' noexpandx
			Button save-template '' 'tooltip=Save current components as template' noexpandx image=save
)foodelim");

	auto component_list = (xhui::ListView*)get_control("components");
	component_list->column_factories[0].f_create = [this](const string& id) -> xhui::Control* {
		return new ComponentPanel(mode_world->data);
	};
	component_list->column_factories[0].f_set = [this](xhui::Control* c, const string& t) {
		const auto xx = t.explode(":");
		reinterpret_cast<ComponentPanel*>(c)->update(xx[0]._int(), xx[1], xx[2]._int());
	};
	component_list->column_factories[0].f_select = [this](xhui::Control* c, bool selected) {
		reinterpret_cast<ComponentPanel*>(c)->set_selected(selected);
	};

	mode_world->data->out_component_added >> create_sink([this] {
		update(true);
	});
	mode_world->data->out_component_removed >> create_sink([this] {
		update(true);
	});
	mode_world->multi_view->out_selection_changed >> create_sink([this] {
		update(false);
	});

	event("add-component", [this] {
		ComponentSelectionDialog::ask(this, mode_world->session).then([this] (const kaba::Class* c) {
			auto e = mode_world->data->entity(cur_entity_index);
			mode_world->data->entity_add_component_generic(e, c);
			//else
			//	mode_world->data->entity_add_user_component(cur_index, c);
		});
	});
	event("save-template", [this] {
		xhui::FileSelectionDialog::ask(this, "Save template", mode_world->session->storage->get_root_dir(FD_MODEL), {"filter=*.template", "save"}).then([this] (const Path& p) {
			Template t;
			auto e = mode_world->data->entity(cur_entity_index);
			for (auto c: e->components)
				if (c->component_type != EdwardTag::_class)
					t.components.add(mode_world->session->plugin_manager->describe_class(c->component_type, c));
			mode_world->session->resource_manager->save_template(&t, p);
		});
	});
	event_x("components", xhui::event_id::Select, [this] {
		int n = get_int("components");
		if (n >= 1 and n <= component_type_list.num and cur_entity_index >= 0)
			preferred_type = component_type_list[n-1];
	});

	/*mode->data->out_changed >> create_sink([this] {
	});*/
}

void EntityPanel::update_to_entity(int index, bool force) {
	if (index == cur_entity_index and !force)
		return;
	cur_entity_index = index;
	reset("components");
	component_type_list.clear();
	auto e = mode_world->data->entity(cur_entity_index);

	add_string("components", format("%d:e:0", cur_entity_index)); // Entity...
	for (const auto& [j, c]: enumerate(e->components))
		if (c->component_type != EdwardTag::_class) {
			add_string("components", format("%d:c:%d", cur_entity_index, j));
			component_type_list.add(c->component_type);
		}
	for (int j=0; j<e->get_component<EdwardTag>()->unknown_components.num; j++)
		add_string("components", format("%d:u:%d", cur_entity_index, j));

	int preferred_component_index = max(component_type_list.find(preferred_type), 0);

	// auto select a "useful" component
	if (e->components.num >= 1)
		set_int("components", preferred_component_index + 1);
	else
		set_int("components", 0);
}

void EntityPanel::update(bool force) {
	const auto& sel = mode_world->multi_view->selection;

	if (sel[MultiViewType::WORLD_ENTITY].num == 0) {
		cur_entity_index = -1;
		reset("components");
	} else if (sel[MultiViewType::WORLD_ENTITY].num == 1) {
		int next = sel[MultiViewType::WORLD_ENTITY][0];
		update_to_entity(next, force);
	} else {
		cur_entity_index = -1;
		reset("components");
	}
}




