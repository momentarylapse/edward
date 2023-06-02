/*
 * NewProjectDialog.cpp
 *
 *  Created on: 25.11.2020
 *      Author: michi
 */

#include "NewProjectDialog.h"

NewProjectDialog::NewProjectDialog(hui::Window *parent) : hui::Dialog("new-project-dialog", parent) {
	_template = 0;
	ok = false;

	add_string("template", "empty");
	add_string("template", "3rd person RPG");
	add_string("template", "space simulation");

	enable("create", false);

	event("find-directory", [this] {
		hui::file_dialog_dir(hui::CurWindow, _("Choose a directory for the new project"), "", {}, [this] (const Path &path) {
			if (path) {
				directory = path;
				set_string("directory", directory.str());
				update();
			}
		});
	});
	event("first-world", [this] {
		update();
	});
	event("cancel", [this] {
		request_destroy();
	});
	event("create", [this] {
		ok = true;
		request_destroy();
	});
}

void NewProjectDialog::update() {
	first_world = get_string("first-world");
	msg_write(format("update  %d  %d", first_world.num, (int)directory.is_empty()));
	enable("create", first_world.num > 0 and !directory.is_empty());
}

