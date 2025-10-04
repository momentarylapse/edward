//
// Created by michi on 04/10/2025.
//

#include "WorldSidePanel.h"
#include "EntityListPanel.h"
#include "EntityPanel.h"
#include "AddEntityPanel.h"
#include "../ModeWorld.h"
#include <view/MultiView.h>
#include <view/DocumentSession.h>

WorldSidePanel::WorldSidePanel(ModeWorld* _mode) : obs::Node<xhui::Panel>("world-side-panel") {
	mode_world = _mode;
	from_source(R"foodelim(
Dialog entity-panel '' padding=0
	Grid main-grid '' expandx spacing=0
)foodelim");

	add_entity_panel = new AddEntityPanel(mode_world);
	entity_list_panel = new EntityListPanel();
	entity_panel = new EntityPanel(mode_world);

	embed("main-grid", 0, 0, add_entity_panel);

	mode_world->multi_view->out_selection_changed >> create_sink([this] {
		update(false);
	});
}

void WorldSidePanel::update(bool force) {
	const auto& sel = mode_world->multi_view->selection;

	auto switch_to = [this] (xhui::Panel* p) {
		if (current_panel and current_panel != p) {
			unembed(current_panel);
		}
		current_panel = p;
		embed("main-grid", 0, 0, p);
	};

	if (sel[MultiViewType::WORLD_ENTITY].num == 0) {
		switch_to(add_entity_panel.get());
	} else if (sel[MultiViewType::WORLD_ENTITY].num == 1) {
		switch_to(entity_panel.get());
		entity_panel.to<EntityPanel>()->update(mode_world);
	} else {
		switch_to(entity_list_panel.get());
		entity_list_panel.to<EntityListPanel>()->update(mode_world);
	}
}