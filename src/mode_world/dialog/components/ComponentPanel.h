//
// Created by michi on 10/1/25.
//

#pragma once

#include <lib/xhui/Panel.h>
#include <lib/obs/Observable.h>

class DataWorld;
namespace kaba {
	struct Class;
}

class ComponentContentsPanel : public obs::Node<xhui::Panel> {
public:
	ComponentContentsPanel(DataWorld* data, int index);
	virtual void update_ui() {}

	DataWorld* data;
	int index;
	bool editing = false;
};

class ComponentPanel : public xhui::Panel {
public:
	explicit ComponentPanel(DataWorld* _data);
	void update(int _entity_index, const string& category, int _component_index);
	void set_class(const kaba::Class* _class);
	void set_class(const string& _component_class);
	void set_selected(bool select);

	DataWorld* data;
	int entity_index = -1;
	int component_index = -1;
	const kaba::Class* component_type = nullptr;
	string component_class;
	ComponentContentsPanel* content_panel = nullptr;
	bool user_component = false;
	bool unknown_component = false;
};

