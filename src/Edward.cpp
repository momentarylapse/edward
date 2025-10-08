

#include <Session.h>
#include <helper/ResourceManager.h>
#include <view/EdwardWindow.h>
#include <view/DocumentSession.h>
#include <stuff/PluginManager.h>
#include <storage/Storage.h>
#include <storage/format/Format.h>
#include <mode_project/ModeProject.h>
#include <lib/os/CommandLineParser.h>
#include <lib/os/msg.h>
#include <lib/kaba/lib/lib.h>
#include <lib/yrenderer/MaterialManager.h>
#include <lib/syntaxhighlight/Theme.h>
#include <mode_material/data/DataMaterial.h>
#include <mode_model/data/DataModel.h>
#include <mode_world/data/DataWorld.h>
#include <test/UnitTest.h>

#include <lib/any/conversion.h>
#include <mode_model/data/ModelMesh.h>
#include <world/LevelData.h>

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

	auto session = create_session(false);
	auto storage = session->storage;
	auto doc = session->create_doc();


	auto _filename = filename.absolute().canonical();

	storage->guess_root_directory(_filename);
	session->resource_manager->shader_manager->shader_dir = storage->root_dir_kind[FD_MATERIAL];
	//msg_write(storage->root_dir.str());

	/*int pp = filename.str().find("/Objects/", 0);
	if (pp > 0) {
		kaba::config.directory = Path(filename.str().sub(0, pp)) | "Scripts";
		//msg_write(kaba::config.directory.str());
	}*/


	Data *data = nullptr;
	string ext = filename.extension();

	if (ext == "shader") {
		auto mat = new DataMaterial(doc);
		msg_write("loading " + filename.str());
		auto &s = mat->appearance.passes[0].shader;
		s.file = _filename.relative_to(storage->root_dir_kind[FD_MATERIAL]);
		s.load_from_file(doc);
		if (s.from_graph)
			s.save_to_file(doc);
		delete mat;
		delete storage;
		delete session;
		return;
	}


	if (ext == "model") {
		//MaterialInit();
		//session->ctx->material_manager->set_default(new yrenderer::Material(session->ctx));
		data = new DataModel(doc);
		DataModelAllowUpdating = false;
	} else if (ext == "material") {
		data = new DataMaterial(doc);
	} else if (ext == "world") {
		data = new DataWorld(doc);
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

// only temporarily needed...
void templatify(const Path &filename) {
	auto session = create_session(false);
	auto storage = session->storage;
	auto doc = session->create_doc();


	auto _filename = filename.absolute().canonical();

	//storage->guess_root_directory(_filename);
	//session->resource_manager->shader_manager->shader_dir = storage->root_dir_kind[FD_MATERIAL];
	//msg_write(storage->root_dir.str());

	/*int pp = filename.str().find("/Objects/", 0);
	if (pp > 0) {
		kaba::config.directory = Path(filename.str().sub(0, pp)) | "Scripts";
		//msg_write(kaba::config.directory.str());
	}*/


	string ext = filename.extension();

	if (ext == "model") {
		//MaterialInit();
		//session->ctx->material_manager->set_default(new yrenderer::Material(session->ctx));
		auto data = new DataModel(doc);
		storage->load(_filename, data, false);
		DataModelAllowUpdating = false;

	    LevelData::Template t;
		{
		    ScriptInstanceData c;
		    c.class_name = "Model";
		    c.set("file", "", str(_filename.basename()));
		    t.components.add(c);
		}
		if (data->phys_mesh->vertices.num > 0) {
		    t.components.add({"MeshCollider"});
		}
	    if (data->meta_data.passive_physics) {
	    	auto T = data->meta_data.inertia_tensor;
	        ScriptInstanceData c;
	        c.class_name = "SolidBody";
	        c.set("physics_active", "", b2s(data->meta_data.active_physics));
	        c.set("mass", "", f2s(data->meta_data.mass, 3));
	        c.set("theta0", "", mat3_to_any(T).str());
	        t.components.add(c);
	    }
		if (data->bones.num > 0) {
	        t.components.add({"Skeleton"});
	    }
		if (data->moves.num > 0) {
	        t.components.add({"Animator"});
	    }
	    LevelData::save_template(t, str(_filename).replace(".model", ".template"));
	}

	delete session;
}

namespace os::app {
int main(const Array<string>& args) {
	xhui::color_space_input = ColorSpace::SRGB;
	xhui::color_space_shaders = ColorSpace::Linear;
	xhui::color_space_display = ColorSpace::SRGB;

	try {
		xhui::init(args, "edward");
	} catch (Exception &e) {
		msg_error(e.message());
		return 1;
	}

	kaba::init();
	syntaxhighlight::init();
	syntaxhighlight::default_theme = syntaxhighlight::get_theme("dark2");


	CommandLineParser p;
	p.info(AppName, "");
	p.cmd("execute", "FILENAME", "(NOT WORKING) execute a script", [] (const Array<string>& args) {
		auto session = create_session();
		session->plugin_manager->execute(Path(args[0]).absolute());
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
	p.cmd("@hidden templatify", "FILENAME", "convert to new template system", [] (const Array<string> &arg) {
		templatify(arg[0]);
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
		DocumentSession doc(&session);
		ModeProject mode_project(&doc);
		mode_project.create_project(arg[0], arg[1]);
	});
	p.cmd("project upgrade", "DIR", "upgrade scripts of a project", [] (const Array<string> &arg) {
		Session session;
		DocumentSession doc(&session);
		ModeProject mode_project(&doc);
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
}

