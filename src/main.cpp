/*
 * main.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"
#include "Edward.h"

#include "Data/Model/DataModel.h"
#include "Data/Material/DataMaterial.h"
#include "Data/World/DataWorld.h"
#include "Storage/Storage.h"

string AppVersion = "0.4.-1.6";
string AppName = "Edward";


EdwardApp::EdwardApp() :
	hui::Application("edward", "English", 0)
{
	app = this;
	set_property("name", AppName);
	set_property("version", AppVersion);
	set_property("copyright", "© 2006-2021 by MichiSoft TM"); // ??? min=2004 (objectmaker) max=2006
	set_property("comment", _("Editor for game data of the y engine"));
	set_property("website", "http://michi.is-a-geek.org/software");
	set_property("author", "Michael Ankele <michi@lupina.de>");
}

#include "lib/kaba/kaba.h"

extern bool DataModelAllowUpdating;
bool handle_special_args(const Array<string> &arg) {
	if (arg[1] == "--help") {
		msg_write("call:");
		msg_write("  FILE");
		msg_write("  --help");
		msg_write("  --check FILE");
		msg_write("  --update FILE");
		msg_write("  --update-model FILE   (deprecated?)");
		return true;
	} else if ((arg[1] == "--update") or (arg[1] == "--check")) {
		if (arg.num >= 3){

			kaba::init();

			int pp = arg[2].find("/Objects/", 0);
			if (pp > 0) {
				kaba::config.directory = Path(arg[2].sub(0, pp)) << "Scripts";
				//msg_write(kaba::config.directory.str());
			}


			Data *data = nullptr;
			string ext = Path(arg[2]).extension();
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
				msg_write("loading " + arg[2]);
				storage->load(arg[2], data, false);
				if (arg[1] == "--update")
					storage->save(arg[2], data);
				delete data;
				return true;
			}
		}
	} else if (arg[1] == "--update-model") {
		if (arg.num >= 3) {
			DataModel m;
			storage->load(arg[2], &m, false);
			//m.save(arg[2]);
		}
		return true;
	}
	return false;

}

bool EdwardApp::on_startup(const Array<string> &arg) {
	if (arg.num >= 2)
		if (handle_special_args(arg))
			return false;

	msg_write(AppName + " " + AppVersion);
	msg_write("");

	ed = new Edward(arg);
	return true;
}

HUI_EXECUTE(EdwardApp)
