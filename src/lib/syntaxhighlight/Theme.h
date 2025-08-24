/*
 * Theme.h
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#pragma once

#include <lib/image/color.h>
#include "BaseParser.h"

namespace syntaxhighlight {

	struct Context {
		color fg = Black;
		color bg = White;
		bool set_bg = false;
		bool bold = false;
		bool italic = false;
	};

	struct Theme {
		string name;
		color bg;
		Context context[(int)MarkupType::NUM_TYPES];
		bool is_default, changed;
		Theme();
		Theme *copy(const string &name);
	};

	void init();
	Theme *get_theme(const string &name);
	Array<Theme*> get_all_themes();
	extern Theme *default_theme;
}
