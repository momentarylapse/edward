//
// Created by michi on 8/24/25.
//

#include "ScriptingPanel.h"
#include "mode_world/ModeWorld.h"
#include "storage/Storage.h"
#include <lib/xhui/controls/Edit.h>
#include <lib/syntaxhighlight/BaseParser.h>
#include <lib/syntaxhighlight/Theme.h>

#include "lib/os/msg.h"


ScriptingPanel::ScriptingPanel(ModeWorld* _mode) : obs::Node<xhui::Panel>("scripting-panel") {
	mode_world = _mode;
	from_source(R"foodelim(
Dialog entity-panel ''
	Grid main-grid ''
		Grid ? '' noexpandx
			ListView systems "systems" expandy width=250
			---|
			FileSelector all-files '' expandy
		MultilineEdit edit0 '' expandx expandy
)foodelim");
	//TabControl open-files 'x' width=800 noexpandx nobar
	//	Edit edit0 ''
	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;
	min_width_user = 320 + 800;

	set_options("all-files", "filter=*.kaba");
	set_options("all-files", "directory=" + str(mode_world->session->storage->root_dir_kind[FD_SCRIPT]));

	fill_list();

	set_target("open-files");
	string id0 = "edit0";
//	add_control("Edit", "", 0, 0, id0);
	set_options(id0, "monospace,fontsize=16,altbg,focusframe=true,lineheightscale=1.1");
	add_string(id0, os::fs::read_text(mode_world->session->storage->root_dir_kind[FD_SCRIPT] | mode_world->data->meta_data.systems[0].filename));
	if (auto p = GetParser("a.kaba")) {
		auto edit = static_cast<xhui::Edit*>(get_control(id0));
		auto theme = syntaxhighlight::default_theme;
		auto markup = p->create_markup(get_string(id0), 0);
		for (const auto& m: markup) {
			const auto& c = theme->context[(int)m.type];
			edit->add_markup({m.start, m.end, c.bold ? xhui::FontFlags::Bold : xhui::FontFlags::None, c.fg});
		}
	}
}

void ScriptingPanel::fill_list() {
	reset("systems");
	for (const auto& s: mode_world->data->meta_data.systems)
		add_string("systems", format("%s - %s", s.class_name, s.filename));
}

