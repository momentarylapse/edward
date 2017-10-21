/*
 * main.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"
#include "Edward.h"

string AppVersion = "0.4.-1.0";
string AppName = "Edward";



EdwardApp::EdwardApp() :
	hui::Application(AppName, "Deutsch", 0)
{
	app = this;
	setProperty("name", AppName);
	setProperty("version", AppVersion);
	setProperty("copyright", "© 2006-2016 by MichiSoft TM"); // ??? min=2004 (objectmaker) max=2006
	setProperty("comment", _("Editor f&ur Spiele-Daten der x9-Engine"));
	setProperty("website", "http://michi.is-a-geek.org/software");
	setProperty("author", "Michael Ankele <michi@lupina.de>");
}

bool EdwardApp::onStartup(const Array<string> &arg)
{
	msg_db_r("main",1);
	msg_write(AppName + " " + AppVersion);
	msg_write("");

	ed = new Edward(arg);
	return true;
}

HUI_EXECUTE(EdwardApp)
