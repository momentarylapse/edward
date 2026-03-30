//
// Created by michi on 10/1/25.
//

#pragma once

#include <lib/xhui/Panel.h>
//#include <lib/pattern/Observable.h>

class DataWorld;

class EntityBasePanel : public xhui::Panel {
public:
	explicit EntityBasePanel(DataWorld* _data, int _index);
	void update_ui();
	void on_edit();
	DataWorld* data;
	int index;
};
