//
// Created by michi on 8/24/25.
//

#include "ScriptingPanel.h"

#include "mode_world/ModeWorld.h"


ScriptingPanel::ScriptingPanel(ModeWorld* _mode) : obs::Node<xhui::Panel>("scripting-panel") {
	mode_world = _mode;
	from_source(R"foodelim(
Dialog entity-panel ''
	Grid main-grid '' expandx
		ListView systems "systems"
)foodelim");
	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;
	min_width_user = 320;

	fill_list();
}

void ScriptingPanel::fill_list() {
	reset("script_list");
	for (const auto& s: mode_world->data->meta_data.systems)
		add_string("systems", format("%s - %s", s.class_name, s.filename));
}

