//
// Created by Michael Ankele on 2025-02-11.
//

#pragma once

#include "Grid.h"


namespace layout {
	struct Resource;
}

namespace xhui {

using Resource = layout::Resource;

class Toolbar : public Grid {
public:
	explicit Toolbar(const string& id);

	void add_item(const string& id, const string& title, const string& image = "");
	void add_item_checkable(const string& id, const string& title, const string& image = "");
	void add_separator();
	void reset() override;
	void set_option(const string& key, const string& value) override;
	void _draw(Painter* p) override;

	void set_by_id(const string &id);
	void from_resource(Resource *res);

	bool is_main = false;
};

} // xhui
