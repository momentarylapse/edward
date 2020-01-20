/*
 * FormatModelPly.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatModelPly.h"
#include "../../Edward.h"

FormatModelPly::FormatModelPly() : TypedFormat<DataModel>(FD_MODEL, "ply", _("Model ply"), Flag::READ) {
}

void FormatModelPly::_load(const string &filename, DataModel *m, bool deep) {
	m->reset();
	m->action_manager->enable(false);
	File *f = NULL;
	bool ok = true;

	try{

	f = FileOpenText(filename);

	// header
	while(true){
		string s = f->read_str();
		if (s == "ply"){
			continue;
		}else if (s.head(7) == "comment"){
			continue;
		}else if (s == "format ascii 1.0"){
			continue;
		}else if (s == "end_header"){
			break;
		}else if (s.head(7) == "element"){
			Array<string> el = s.explode(" ");
			if (el.num != 3)
				throw Exception("element can't be parsed: " + s);
		}else if (s.head(8) == "property"){

		}else{
			throw Exception("don't understand header line: " + s);
		}
	}

	}catch(Exception &e){
		msg_error(e.message());
		ok = false;
	}

	if (f)
		delete(f);

	m->action_manager->enable(true);
}

void FormatModelPly::_save(const string &filename, DataModel *data) {
}
