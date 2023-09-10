/*
 * Edward.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Edward.h"
#include "EdwardWindow.h"
#include "lib/os/CommandLineParser.h"
#include "lib/hui/hui.h"
#include "lib/kaba/kaba.h"
#include "data/model/DataModel.h"
#include "data/material/DataMaterial.h"
#include "data/world/DataWorld.h"
#include "storage/Storage.h"
#include "stuff/PluginManager.h"
#include "mode/administration/ModeAdministration.h"

string AppVersion = "0.4.-1.7";
string AppName = "Edward";

EdwardApp *app = nullptr;


EdwardApp::EdwardApp() :
	hui::Application("edward", "English", hui::Flags::NONE)
{
	app = this;
	set_property("name", AppName);
	set_property("version", AppVersion);
	set_property("copyright", "© 2006-2023 by MichiSoft TM"); // ??? min=2004 (objectmaker) max=2006
	set_property("comment", _("Editor for game data of the y engine"));
	set_property("website", "http://michi.is-a-geek.org/software");
	set_property("author", "Michael Ankele <michi@lupina.de>");


	// depends on some mode data
	if (app->installed)
		plugins = new PluginManager(app->directory_static | "plugins");
	else
		plugins = new PluginManager(app->directory_static | ".." | "plugins");
}

extern bool DataModelAllowUpdating;
void update_file(const Path &filename, bool allow_write) {

	kaba::init();
	EdwardWindow ed;

	int pp = filename.str().find("/Objects/", 0);
	if (pp > 0) {
		kaba::config.directory = Path(filename.str().sub(0, pp)) | "Scripts";
		//msg_write(kaba::config.directory.str());
	}


	Data *data = nullptr;
	string ext = filename.extension();
	if (ext == "model") {
		//MaterialInit();
		ed.material_manager->set_default(new Material(ed.resource_manager));
		data = new DataModel(&ed);
		DataModelAllowUpdating = false;
	} else if (ext == "material") {
		data = new DataMaterial(&ed, false);
	} else if (ext == "world") {
		data = new DataWorld(&ed);
	}

	if (data) {
		auto storage = new Storage(&ed);
		msg_write("loading " + filename.str());
		storage->load(filename, data, false);
		if (allow_write)
			storage->save(filename, data);
		delete data;
	}
}

void test_gl();

hui::AppStatus EdwardApp::on_startup(const Array<string> &arg) {
	CommandLineParser p;
	p.info(AppName, "");
	p.option("--execute", "FILENAME", "(NOT WORKING) execute a script", [] (const string &a) {
		//plugins->execute(a);
	});
	p.cmd("-h/--help", "", "show this help page", [&p] (const Array<string> &arg) {
		p.show();
	});
	p.cmd("file update", "FILENAME", "load and re-write a file", [] (const Array<string> &arg) {
		update_file(arg[0], true);
	});
	p.cmd("file check", "FILENAME", "load file and check for errors", [] (const Array<string> &arg) {
		update_file(arg[0], false);;
	});
	p.cmd("new material", "", "open editor in material mode", [] (const Array<string> &arg) {
		auto ed = new EdwardWindow();
		ed->universal_new(FD_MATERIAL);
	});
	p.cmd("new font", "", "open editor in font mode", [] (const Array<string> &arg) {
		auto ed = new EdwardWindow();
		ed->universal_new(FD_FONT);
	});
	p.cmd("new world", "", "open editor in world mode", [] (const Array<string> &arg) {
		auto ed = new EdwardWindow();
		ed->universal_new(FD_WORLD);
	});
	p.cmd("project create", "DIR FIRST_WORLD", "create a new project", [] (const Array<string> &arg) {
		EdwardWindow ed;
		ModeAdministration mode_admin(&ed);
		mode_admin.create_project(arg[0], arg[1]);
	});
	p.cmd("project upgrade", "DIR", "upgrade scripts of a project", [] (const Array<string> &arg) {
		EdwardWindow ed;
		ModeAdministration mode_admin(&ed);
		mode_admin.upgrade_project(arg[0]);
	});
	p.cmd("gl", "", "", [] (const Array<string> &arg) {
			test_gl();
	});
	p.cmd("", "[FILENAME]", "open editor and load a file", [] (const Array<string> &arg) {
		msg_write(AppName + " " + AppVersion);
		msg_write("");

		auto ed = new EdwardWindow();

		if (arg.num > 0) {
			int type = ed->storage->guess_type(arg[0]);

			if (type >= 0) {
				ed->universal_edit(type, arg[0], false);
			} else {
				ed->error_box(_("Unknown file extension: ") + arg[0]);
				app->end();
			}
		} else {
			ed->universal_new(FD_MODEL);
		}
	});
	p.parse(arg);


	/*if (arg.num >= 2)
		if (handle_special_args(arg))
			return false;*/

	return hui::AppStatus::AUTO;
}

HUI_EXECUTE(EdwardApp)
