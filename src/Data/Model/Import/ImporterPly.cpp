/*
 * ImporterPly.cpp
 *
 *  Created on: Jun 10, 2015
 *      Author: ankele
 */

#include "ImporterPly.h"

ImporterPly::ImporterPly()
{
}

ImporterPly::~ImporterPly()
{
}


bool ImporterPly::Import(DataModel *m, const string &filename)
{
	m->reset();
	m->action_manager->enable(false);
	f = NULL;
	bool ok = true;

	try{

	f = FileOpen(filename);
	if (!f)
		throw string("can't open file");
	f->SetBinaryMode(false);

	// header
	while(true){
		string s = f->ReadStr();
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
				throw string("element can't be parsed: ") + s;
		}else if (s.head(8) == "property"){

		}else{
			throw string("don't understand header line: ") + s;
		}
	}

	}catch(string &e){
		msg_error(e);
		ok = false;
	}

	if (f)
		delete(f);

	m->action_manager->enable(true);
	return ok;
}
