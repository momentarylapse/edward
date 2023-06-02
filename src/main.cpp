/*
 * main.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Edward.h"
#include "lib/os/CommandLineParser.h"
#include "lib/hui/hui.h"
#include "lib/kaba/kaba.h"
#include "Data/Model/DataModel.h"
#include "Data/Material/DataMaterial.h"
#include "Data/World/DataWorld.h"
#include "Storage/Storage.h"
#include "Mode/Administration/ModeAdministration.h"

string AppVersion = "0.4.-1.6";
string AppName = "Edward";


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
}

extern bool DataModelAllowUpdating;
void update_file(const Path &filename, bool allow_write) {

	kaba::init();

	int pp = filename.str().find("/Objects/", 0);
	if (pp > 0) {
		kaba::config.directory = Path(filename.str().sub(0, pp)) | "Scripts";
		//msg_write(kaba::config.directory.str());
	}


	Data *data = nullptr;
	string ext = filename.extension();
	if (ext == "model") {
		//MaterialInit();
		SetDefaultMaterial(new Material);
		data = new DataModel;
		DataModelAllowUpdating = false;
	} else if (ext == "material") {
		data = new DataMaterial(false);
	} else if (ext == "world") {
		data = new DataWorld;
	}

	if (data) {
		storage = new Storage();
		msg_write("loading " + filename.str());
		storage->load(filename, data, false);
		if (allow_write)
			storage->save(filename, data);
		delete data;
	}
}

bool EdwardApp::on_startup(const Array<string> &arg) {
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
		ed = new Edward();
		ed->universal_new(FD_MATERIAL);
	});
	p.cmd("new font", "", "open editor in font mode", [] (const Array<string> &arg) {
		ed = new Edward();
		ed->universal_new(FD_FONT);
	});
	p.cmd("new world", "", "open editor in world mode", [] (const Array<string> &arg) {
		ed = new Edward();
		ed->universal_new(FD_WORLD);
	});
	p.cmd("project create", "DIR FIRST_WORLD", "create a new project", [] (const Array<string> &arg) {
		ModeAdministration mode_admin;
		mode_admin.create_project(arg[0], arg[1]);
	});
	p.cmd("project upgrade", "DIR", "upgrade scripts of a project", [] (const Array<string> &arg) {
		ModeAdministration mode_admin;
		mode_admin.upgrade_project(arg[0]);
	});
	p.cmd("", "[FILENAME]", "open editor and load a file", [] (const Array<string> &arg) {
		msg_write(AppName + " " + AppVersion);
		msg_write("");

		ed = new Edward();

		if (arg.num > 0) {
			int type = storage->guess_type(arg[0]);

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

	return ed;
}

HUI_EXECUTE(EdwardApp)
