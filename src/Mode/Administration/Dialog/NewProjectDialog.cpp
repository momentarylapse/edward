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

	event("find-directory", [=]{
		if (hui::FileDialogDir(hui::CurWindow, _("Choose a directory for the new project"), "")) {
			directory = hui::Filename;
			set_string("directory", directory.str());
			enable("create", true);
		}
	});
	event("cancel", [=]{
		request_destroy();
	});
	event("create", [=]{
		ok = true;
		request_destroy();
	});
}

NewProjectDialog::~NewProjectDialog() {
}

