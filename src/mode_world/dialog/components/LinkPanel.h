//
// Created by michi on 3/30/26.
//

#pragma once

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

class DataWorld;

class LinkPanel : public obs::Node<xhui::Panel> {
public:
	explicit LinkPanel(DataWorld* _data, int _index);
	DataWorld* data;
	int index;
	bool editing = false;

	void update_ui();
	void on_edit();
};
