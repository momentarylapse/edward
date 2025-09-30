//
// Created by michi on 02.02.25.
//

#include "EntityPanel.h"
#include "AddEntityPanel.h"
#include "ComponentPanel.h"
#include "../ModeWorld.h"
#include "ComponentSelectionDialog.h"
#include <stuff/PluginManager.h>
#include <lib/base/iter.h>
#include <lib/xhui/controls/ListView.h>
#include <view/MultiView.h>
#include <view/DocumentSession.h>
#include <y/EntityManager.h>
#include <y/Entity.h>
#include <y/world/Camera.h>
#include <y/world/Light.h>
#include <y/world/Model.h>
#include <y/world/Terrain.h>

class EntityListPanel : public xhui::Panel {
public:
	explicit EntityListPanel() : Panel("entity-list-panel") {
		from_source(R"foodelim(
Dialog entity-base-panel ''
	Grid card-entity-list '' class=card
		Group group-entity-list 'Selection'
			ListView list 'x' nobar
)foodelim");
	}
	void update(ModeWorld* mode) {
		reset("list");
		const auto& sel = mode->multi_view->selection[MultiViewType::WORLD_ENTITY];
		for (const auto& [i,e]: enumerate(mode->data->entity_manager->get_component_list<EdwardTag>()))
			if (sel.contains(i)) {
				string name = "Entity";
				if (e->owner->get_component<Camera>())
					name = "Camera";
				if (e->owner->get_component<Light>())
					name = "Light";
				if (e->owner->get_component<TerrainRef>())
					name = "Terrain";
				if (auto r = e->owner->get_component<ModelRef>()) {
					name = "Model (no file)";
					if (r->model)
						name = "Model " + str(r->filename);
				}
				add_string("list", name);
			}
	}
};

EntityPanel::EntityPanel(ModeWorld* _mode) : obs::Node<xhui::Panel>("entity-panel") {
	mode_world = _mode;
	from_source(R"foodelim(
Dialog entity-panel '' padding=0
	Grid main-grid '' expandx spacing=0
		.
		---|
		ListView components 'c' nobar sunkenbackground=no showselection=no padding=7 selectsingle hidden
		---|
		Button add-component '+' hidden
)foodelim");

	add_entity_panel = new AddEntityPanel(mode_world);
	embed("main-grid", 0, 0, add_entity_panel);

	entity_list_panel = new EntityListPanel();

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

void EntityPanel::update(bool force) {
	const auto& sel = mode_world->multi_view->selection;

	unembed(add_entity_panel.get());
	unembed(entity_list_panel.get());

	if (sel[MultiViewType::WORLD_ENTITY].num == 0) {
		cur_index = -1;
		reset("components");
		set_visible("components", false);
		set_visible("add-component", false);
		if (!add_entity_panel->owner)
			embed("main-grid", 0, 0, add_entity_panel);
	} else if (sel[MultiViewType::WORLD_ENTITY].num == 1) {
		int next = sel[MultiViewType::WORLD_ENTITY][0];
		if (next == cur_index and !force)
			return;
		cur_index = next;
		reset("components");
		auto e = mode_world->data->entity_manager->entities[cur_index];
		set_options("components-viewport", "expandy");
		set_visible("components", true);
		set_visible("add-component", true);

		add_string("components", format("%d:e:0", cur_index)); // Entity...
		for (int j=0; j<e->components.num; j++)
			if (e->components[j]->component_type != EdwardTag::_class)
				add_string("components", format("%d:c:%d", cur_index, j));
		for (int j=0; j<e->get_component<EdwardTag>()->unknown_components.num; j++)
			add_string("components", format("%d:u:%d", cur_index, j));

		if (e->components.num >= 1)
			set_int("components", 1);
		else
			set_int("components", 0);
	} else {
		cur_index = -1;
		reset("components");
		set_visible("components", false);
		set_visible("add-component", false);
		if (!entity_list_panel->owner)
			embed("main-grid", 0, 0, entity_list_panel);
		entity_list_panel.to<EntityListPanel>()->update(mode_world);
	}
}




