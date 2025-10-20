//
// Created by michi on 04/10/2025.
//

#include "EntityListPanel.h"
#include "../ModeWorld.h"
#include <lib/base/iter.h>
#include <view/MultiView.h>
#include <view/DocumentSession.h>
#include <ecs/EntityManager.h>
#include <ecs/Entity.h>
#include <y/world/components/Camera.h>
#include <y/world/components/Light.h>
#include <y/world/Model.h>
#include <y/world/Terrain.h>


EntityListPanel::EntityListPanel() : Panel("entity-list-panel") {
	from_source(R"foodelim(
Dialog entity-base-panel ''
	Grid card-entity-list '' class=card
		Group group-entity-list 'Selection'
			ListView list 'x' nobar
)foodelim");
}
void EntityListPanel::update(ModeWorld* mode) {
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
