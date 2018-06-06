/*
 * main.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"
#include "Edward.h"

#include "Data/Model/DataModel.h"

string AppVersion = "0.4.-1.2";
string AppName = "Edward";


EdwardApp::EdwardApp() :
	hui::Application("edward", "Deutsch", hui::FLAG_LOAD_RESOURCE)
{
	app = this;
	setProperty("name", AppName);
	setProperty("version", AppVersion);
	setProperty("copyright", "© 2006-2018 by MichiSoft TM"); // ??? min=2004 (objectmaker) max=2006
	setProperty("comment", _("Editor f&ur Spiele-Daten der x9-Engine"));
	setProperty("website", "http://michi.is-a-geek.org/software");
	setProperty("author", "Michael Ankele <michi@lupina.de>");
}

#include "lib/kaba/kaba.h"

extern bool DataModelAllowUpdating;
bool handle_special_args(const Array<string> &arg)
{
	if ((arg[1] == "--update") or (arg[1] == "--check")){
		if (arg.num >= 3){

			Kaba::Init();

			int pp = arg[2].find("/Objects/", 0);
			if (pp > 0){
				Kaba::config.directory = arg[2].substr(0, pp) + "/Scripts/";
				msg_write(Kaba::config.directory);
			}


			string ext = arg[2].extension();
			if (ext == "model"){
				DataModelAllowUpdating = false;
				DataModel m;
				m.load(arg[2], false);
				if (arg[1] == "--update")
					m.save(arg[2]);
				return true;
			}
		}
	}
	return false;

}

bool EdwardApp::onStartup(const Array<string> &arg)
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
