/*
 * main.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"
#include "Edward.h"

string AppVersion = "0.3.99.3";
string AppName = "Edward";

int hui_main(Array<string> arg)
{
	HuiInit("edward", true, "Deutsch");
	HuiSetProperty("name", AppName);
	HuiSetProperty("version", AppVersion);
	HuiSetProperty("copyright", "© 2006-2014 by MichiSoft TM"); // ??? min=2004 (objectmaker) max=2006
	HuiSetProperty("comment", _("Editor f&ur Spiele-Daten der x9-Engine"));
	HuiSetProperty("website", "http://michi.is-a-geek.org/software");
	HuiSetProperty("author", "Michael Ankele <michi@lupina.de>");

	msg_db_r("main",1);
	msg_write(AppName + " " + AppVersion);
	msg_write("");

	ed = new Edward(arg);
	return ed->Run();
}
