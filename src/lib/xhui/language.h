/*----------------------------------------------------------------------------*\
| Hui language                                                                 |
| -> string format conversion                                                  |
| -> translations                                                              |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2011.01.18 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _LANG_EXISTS_
#define _LANG_EXISTS_

#include "../base/base.h"

class Path;



namespace xhui {


extern string get_lang(const string &ns, const string &id, const string &text, bool allow_keys=false);

struct Resource;


// language
Array<string> get_languages();
string get_cur_language();
void _cdecl set_language(const string &language);
extern bool _using_language_;

string _cdecl get_language(const string &ns, const string &id);
string _cdecl get_language_r(const string &ns, const Resource &cmd);
string _cdecl get_language_t(const string &ns, const string &id, const string &tooltip);
string _cdecl get_language_s(const string &str);
#define L(ns, id)	xhui::get_language(ns, id)
#define _(str)	xhui::get_language_s(str)
void _cdecl UpdateAll();

// internal

struct Language {
	struct Translation {
		string orig;
		string trans; // pre defined translation of orig
	};

	struct Command {
		string _namespace, id, text, tooltip;
		bool match(const string &_ns, const string &id);
	};

	string name;
	Array<Command> cmd; // text associated to ids
	Array<Translation> trans;
};

};

#endif
