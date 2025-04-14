/*----------------------------------------------------------------------------*\
| Hui resource                                                                 |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2011.01.18 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_RESOURCE_EXISTS_
#define _HUI_RESOURCE_EXISTS_

#include "../base/base.h"
#include "../base/pointer.h"

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



struct Resource {
	string type;
	string id;
	string title;
	string tooltip;
	Array<string> options;
	int x, y;
	Array<Resource> children;
	Resource();
	Resource* get_node(const string& id) const;
	bool enabled() const;
	string image() const;
	bool has(const string& key) const;
	string value(const string& key, const string& fallback = "") const;
	void show(int indent = 0) const;
	string to_string(int indent = 0) const;
};

Resource parse_resource(const string& buffer, bool literal = false);


// resources
void load_resource(const Path& filename);
Resource* get_resource(const string& id);

};

#endif
