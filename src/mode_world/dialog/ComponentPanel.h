//
// Created by michi on 10/1/25.
//

#pragma once

#include <lib/xhui/Panel.h>
//#include <lib/pattern/Observable.h>

class DataWorld;
namespace kaba {
	class Class;
}

class ComponentPanel : public xhui::Panel {
public:
	explicit ComponentPanel(DataWorld* _data);
	void update(int _entity_index, const string& category, int _component_index);
	void set_class(const string& _component_class);
	void set_selected(bool select);

	DataWorld* data;
	int entity_index = -1;
	int component_index = -1;
	const kaba::Class* component_type;
	string component_class;
	Panel* content_panel = nullptr;
	bool user_component = false;
	bool unknown_component = false;
};

