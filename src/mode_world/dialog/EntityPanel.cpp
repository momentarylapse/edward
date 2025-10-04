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
#include <view/MultiView.h>
#include <view/DocumentSession.h>
#include <y/EntityManager.h>
#include <y/Entity.h>
#include <y/world/Camera.h>

EntityPanel::EntityPanel(ModeWorld* _mode) : obs::Node<xhui::Panel>("entity-panel") {
	mode_world = _mode;
	from_source(R"foodelim(
Dialog entity-panel '' padding=0
	Grid main-grid '' margin=7
		ListView components 'c' nobar sunkenbackground=no showselection=no selectsingle hidden
		---|
		Button add-component '+' hidden
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
			auto e = mode_world->data->entity(cur_index);
			mode_world->data->entity_add_component_generic(e, c);
			//else
			//	mode_world->data->entity_add_user_component(cur_index, c);
		});
	});

	/*mode->data->out_changed >> create_sink([this] {
	});*/
}

void EntityPanel::update_xxx(int next, bool force) {
	if (next == cur_index and !force)
		return;
	cur_index = next;
	reset("components");
	auto e = mode_world->data->entity(cur_index);
	set_options("components-viewport", "expandy");
	set_visible("components", true);
	set_visible("add-component", true);

	add_string("components", format("%d:e:0", cur_index)); // Entity...
	for (int j=0; j<e->components.num; j++)
		if (e->components[j]->component_type != EdwardTag::_class)
			add_string("components", format("%d:c:%d", cur_index, j));
	for (int j=0; j<e->get_component<EdwardTag>()->unknown_components.num; j++)
		add_string("components", format("%d:u:%d", cur_index, j));

	// auto select a "useful" component
	if (e->components.num >= 1)
		set_int("components", 1);
	else
		set_int("components", 0);
}

void EntityPanel::update(bool force) {
	const auto& sel = mode_world->multi_view->selection;

	if (sel[MultiViewType::WORLD_ENTITY].num == 0) {
		cur_index = -1;
		reset("components");
	} else if (sel[MultiViewType::WORLD_ENTITY].num == 1) {
		int next = sel[MultiViewType::WORLD_ENTITY][0];
		update_xxx(next, force);
	} else {
		cur_index = -1;
		reset("components");
	}
}




