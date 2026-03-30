//
// Created by michi on 10/1/25.
//

#pragma once

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

class DataWorld;

class UserComponentPanel : public obs::Node<xhui::Panel> {
public:
	explicit UserComponentPanel(DataWorld* _data, int _index, int _cindex);
	DataWorld* data;
	int index, cindex;
	bool editing = false;

	void update_ui();
	void on_edit();
};

