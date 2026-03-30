//
// Created by michi on 9/30/25.
//

#pragma once

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

class DataWorld;

class CameraPanel : public obs::Node<xhui::Panel> {
public:
	explicit CameraPanel(DataWorld* _data, int _index);
	DataWorld* data;
	int index;
	bool editing = false;

	void update_ui();
	void on_edit();
};
