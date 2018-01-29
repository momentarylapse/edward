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
	return ok;
}
