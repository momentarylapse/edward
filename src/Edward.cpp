

#include <Session.h>
#include <helper/ResourceManager.h>
#include <view/EdwardWindow.h>
#include <stuff/PluginManager.h>
#include <storage/Storage.h>
#include <storage/format/Format.h>
#include <mode_project/ModeProject.h>
#include <lib/os/CommandLineParser.h>
#include <lib/os/msg.h>
#include <lib/kaba/lib/lib.h>
#include <mode_material/data/DataMaterial.h>
#include <mode_model/data/DataModel.h>
#include <mode_world/data/DataWorld.h>
#include <test/UnitTest.h>

string AppVersion = "0.5.-1.0";
string AppName = "Edward";

//EdwardApp *app = nullptr;
void* app = nullptr;

namespace hui {
	string get_language_s(const string& lang) {
		return "";
	}
}

bool any_session_running();

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


int xhui_main(const Array<string>& args) {
	try {
		xhui::init(args, "edward");
	} catch (Exception &e) {
		msg_error(e.message());
		return 1;
	}

	kaba::init();


	CommandLineParser p;
	p.info(AppName, "");
	p.cmd("execute", "FILENAME", "(NOT WORKING) execute a script", [] (const Array<string>& args) {
		auto session = create_session();
		session->plugin_manager->execute(session, Path(args[0]).absolute());
	});
	p.cmd("-h/--help", "", "show this help page", [&p] (const Array<string> &arg) {
		p.show();
	});
	p.cmd("file update", "FILENAME", "load and re-write a file", [] (const Array<string> &arg) {
		update_file(arg[0], true);
	});
	p.cmd("file check", "FILENAME", "load file and check for errors", [] (const Array<string> &arg) {
		update_file(arg[0], false);
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
		ModeProject mode_project(&session);
		mode_project.create_project(arg[0], arg[1]);
	});
	p.cmd("project upgrade", "DIR", "upgrade scripts of a project", [] (const Array<string> &arg) {
		Session session;
		ModeProject mode_project(&session);
		mode_project.upgrade_project(arg[0]);
	});
	p.cmd("xxx", "", "", [] (const Array<string> &arg) {
		/*Session session;
		ModeAdministration mode_admin(&session);*/
		msg_todo("mode admin");
	});
	p.cmd("@hidden test", "", "", [] (const Array<string> &arg) {
		unittest::run_all("*");
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
				session->error("Unknown file extension: " + arg[0]);
				//app->end();
			}
		} else {
			session->universal_new(FD_MODEL);
		}
	});
	p.parse(args);

	try {
		if (any_session_running())
			xhui::run();
	} catch (Exception& e) {
		msg_error(e.message());
	}
	return 0;
}

