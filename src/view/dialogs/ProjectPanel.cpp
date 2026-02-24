//
// Created by michi on 2/24/26.
//

#include "ProjectPanel.h"
#include "Session.h"
#include "../EdwardWindow.h"
#include "storage/Storage.h"

ProjectPanel::ProjectPanel(EdwardWindow *window) : obs::Node<Panel>("project-panel") {
	editor_window = window;
	session = editor_window->session.get();
	from_source(R"foodelim(
Dialog project-panel 'Project' width=250
	Grid ? '' class=card
		Group ? 'Project'
			Grid ? ''
				FileSelector files '' dragsource=project-file expandy
)foodelim");
	xhui::run_later(0.01f, [this] {
		set_options("files", "linkevents");
	});
	session->out_project_loaded >> create_sink([this] {
		set_options("files", format("root=%s", session->project_dir));
		set_options("files", format("directory=%s", session->project_dir));
	});
	event("files", [this] {
		const auto file = get_string("files");
		session->universal_edit(Storage::get_file_kind(file), file, false);
	});
}

ProjectPanel::~ProjectPanel() = default;
