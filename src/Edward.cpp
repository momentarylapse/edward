

#include <Session.h>
#include <view/EdwardWindow.h>
#include <storage/Storage.h>
#include <storage/format/Format.h>
#include <mode_project/ModeProject.h>
#include <lib/os/CommandLineParser.h>
#include <lib/os/msg.h>
#include <lib/kaba/lib/lib.h>

string AppVersion = "0.5.-1.0";
string AppName = "Edward";

//EdwardApp *app = nullptr;
void* app = nullptr;

namespace hui {
	string get_language_s(const string& lang) {
		return "";
	}
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
	p.option("--execute", "FILENAME", "(NOT WORKING) execute a script", [] (const string &a) {
		//plugins->execute(a);
	});
	p.cmd("-h/--help", "", "show this help page", [&p] (const Array<string> &arg) {
		p.show();
	});
	p.cmd("file update", "FILENAME", "load and re-write a file", [] (const Array<string> &arg) {
		//update_file(arg[0], true);
		msg_todo("file update");
	});
	p.cmd("file check", "FILENAME", "load file and check for errors", [] (const Array<string> &arg) {
		//update_file(arg[0], false);
		msg_todo("file check");
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
		xhui::run();
	} catch (Exception& e) {
		msg_error(e.message());
	}
	return 0;
}

