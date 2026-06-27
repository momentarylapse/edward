/*----------------------------------------------------------------------------*\
| Hui resource                                                                 |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2011.01.18 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#pragma once

#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <lib/layout/Resource.h>

class Path;

namespace xhui {

	class Panel;
	class Menu;
	class Dialog;
	class Window;

//----------------------------------------------------------------------------------
// resource handling

Dialog* create_resource_dialog(const string &id, Window *root);
xfer<Menu> create_resource_menu(const string &id);


using Resource = layout::Resource;

Resource parse_resource(const string& buffer, bool literal = false);

Array<layout::Option> parse_options(const string& s);


// resources
void load_resource(const Path& filename);
Resource* get_resource(const string& id);

};
