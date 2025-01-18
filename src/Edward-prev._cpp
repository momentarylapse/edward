/*
 * Edward.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Edward.h"
#include "EdwardWindow.h"
#include "Session.h"
#include <lib/os/CommandLineParser.h>
#include "lib/os/msg.h"
#include <lib/hui/hui.h>
#include <lib/kaba/kaba.h>
#include <y/helper/ResourceManager.h>
#include "data/model/DataModel.h"
#include "data/material/DataMaterial.h"
#include "data/world/DataWorld.h"
#include "storage/Storage.h"
#include "stuff/PluginManager.h"
#include "mode/administration/ModeAdministration.h"

string AppVersion = "0.4.-1.9";
string AppName = "Edward";

EdwardApp *app = nullptr;


EdwardApp::EdwardApp() :
	hui::Application("edward", "English", hui::Flags::NONE)
{
	app = this;
	set_property("name", AppName);
	set_property("version", AppVersion);
	set_property("copyright", "© 2006-2024 by MichiSoft TM"); // ??? min=2004 (objectmaker) max=2006
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
	//Session session;

	auto session = new Session;
	session->resource_manager = new ResourceManager(nullptr);
	auto storage = new Storage(session);


	auto _filename = filename.absolute().canonical();

	storage->guess_root_directory(_filename);
	session->resource_manager->shader_dir = storage->root_dir_kind[FD_MATERIAL];
	//msg_write(storage->root_dir.str());

	/*int pp = filename.str().find("/Objects/", 0);
	if (pp > 0) {
		kaba::config.directory = Path(filename.str().sub(0, pp)) | "Scripts";
		//msg_write(kaba::config.directory.str());
	}*/


	Data *data = nullptr;
	string ext = filename.extension();

	if (ext == "shader") {
		auto mat = new DataMaterial(session);
		msg_write("loading " + filename.str());
		auto &s = mat->appearance.passes[0].shader;
		s.file = _filename.relative_to(storage->root_dir_kind[FD_MATERIAL]);
		s.load_from_file(session);
		if (s.from_graph)
			s.save_to_file(session);
		delete mat;
		delete storage;
		delete session;
		return;
	}


	if (ext == "model") {
		//MaterialInit();
		session->resource_manager->material_manager->set_default(new Material(session->resource_manager));
		data = new DataModel(session);
		DataModelAllowUpdating = false;
	} else if (ext == "material") {
		data = new DataMaterial(session);
	} else if (ext == "world") {
		data = new DataWorld(session);
	}

	if (data) {
		msg_write("loading " + filename.str());
		storage->load(filename, data, false);
		if (allow_write)
			storage->save(filename, data);
		delete data;
	}
	delete storage;
	delete session;
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
		auto session = create_session();
		session->universal_new(FD_MATERIAL);
	});
	p.cmd("new font", "", "open editor in font mode", [] (const Array<string> &arg) {
		auto session = create_session();
		session->universal_new(FD_FONT);
	});
	p.cmd("new world", "", "open editor in world mode", [] (const Array<string> &arg) {
		auto session = create_session();
		session->universal_new(FD_WORLD);
	});
	p.cmd("project create", "DIR FIRST_WORLD", "create a new project", [] (const Array<string> &arg) {
		Session session;
		ModeAdministration mode_admin(&session);
		mode_admin.create_project(arg[0], arg[1]);
	});
	p.cmd("project upgrade", "DIR", "upgrade scripts of a project", [] (const Array<string> &arg) {
		Session session;
		ModeAdministration mode_admin(&session);
		mode_admin.upgrade_project(arg[0]);
	});
	p.cmd("xxx", "", "", [] (const Array<string> &arg) {
		Session session;
		ModeAdministration mode_admin(&session);
	});
	p.cmd("", "[FILENAME]", "open editor and load a file", [] (const Array<string> &arg) {
		msg_write(AppName + " " + AppVersion);
		msg_write("");

		auto session = create_session();

		if (arg.num > 0) {
			int type = session->storage->guess_type(arg[0]);

			if (type >= 0) {
				session->universal_edit(type, arg[0], false);
			} else {
				session->error(_("Unknown file extension: ") + arg[0]);
				app->end();
			}
		} else {
			session->universal_new(FD_MODEL);
		}
	});
	p.parse(arg);


	/*if (arg.num >= 2)
		if (handle_special_args(arg))
			return false;*/

	return hui::AppStatus::AUTO;
}

HUI_EXECUTE(EdwardApp)
