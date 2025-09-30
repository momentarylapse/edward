//
// Created by michi on 9/30/25.
//

#pragma once

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

class DataWorld;


class SolidBodyPanel : public xhui::Panel {
public:
	explicit SolidBodyPanel(DataWorld* _data, int _index);
	void update_ui();
	void on_edit();
	DataWorld* data;
	int index;
};
