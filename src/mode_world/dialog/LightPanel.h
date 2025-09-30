//
// Created by michi on 9/30/25.
//

#pragma once

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

class DataWorld;

class LightPanel : public obs::Node<xhui::Panel> {
public:
	explicit LightPanel(DataWorld* _data, int _index);
	DataWorld* data;
	int index;

	void update_ui();
	void on_edit();
};
