//
// Created by michi on 7/7/26.
//

#include "SettingsDialog.h"
#include <lib/xhui/config.h>
#include <lib/xhui/dialogs/FileSelectionDialog.h>

string nice_path(const Path& path);

SettingsDialog::SettingsDialog(Panel* parent) : Dialog("settings-dialog", parent) {

	set_string("engine-directory", nice_path(xhui::config.get_str("EngineDir", "")));

	event("engine-directory", [this] {
		xhui::FileSelectionDialog::ask(this, "Engine directory", xhui::config.get_str("EngineDir", ""), {"directory"}).then([this] (const Path& path) {
			xhui::config.set_str("EngineDir", str(path));
			set_string("engine-directory", nice_path(path));
		});
	});
}
