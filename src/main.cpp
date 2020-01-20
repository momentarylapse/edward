/*
 * main.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"
#include "Edward.h"

#include "Data/Model/DataModel.h"
#include "Storage/Storage.h"

string AppVersion = "0.4.-1.4";
string AppName = "Edward";


EdwardApp::EdwardApp() :
	hui::Application("edward", "English", hui::FLAG_LOAD_RESOURCE)
{
	app = this;
	set_property("name", AppName);
	set_property("version", AppVersion);
	set_property("copyright", "© 2006-2020 by MichiSoft TM"); // ??? min=2004 (objectmaker) max=2006
	set_property("comment", _("Editor for game data of the x9 engine"));
	set_property("website", "http://michi.is-a-geek.org/software");
	set_property("author", "Michael Ankele <michi@lupina.de>");
}

#include "lib/kaba/kaba.h"

extern bool DataModelAllowUpdating;
bool handle_special_args(const Array<string> &arg)
{
	if ((arg[1] == "--update") or (arg[1] == "--check")){
		if (arg.num >= 3){

			Kaba::init();

			int pp = arg[2].find("/Objects/", 0);
			if (pp > 0){
				Kaba::config.directory = arg[2].substr(0, pp) + "/Scripts/";
				msg_write(Kaba::config.directory);
			}


			string ext = arg[2].extension();
			if (ext == "model"){
				DataModelAllowUpdating = false;
				DataModel m;
				storage->load(arg[2], &m, false);
				if (arg[1] == "--update")
					storage->save(arg[2], &m);
				return true;
			}
		}
	}
	return false;

}

bool EdwardApp::on_startup(const Array<string> &arg)
{
	if (arg.num >= 2)
		if (handle_special_args(arg))
			return false;

	msg_write(AppName + " " + AppVersion);
	msg_write("");

	ed = new Edward(arg);
	return true;
}

HUI_EXECUTE(EdwardApp)
