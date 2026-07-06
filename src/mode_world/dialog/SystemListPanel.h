//
// Created by michi on 7/6/26.
//

#pragma once

#include <lib/xhui/Dialog.h>
#include <lib/pattern/Observable.h>
#include "../data/DataWorld.h"


class SystemListPanel : public obs::Node<xhui::Panel> {
public:
	explicit SystemListPanel(DataWorld* data);
	DataWorld* data;

	DataWorld::MetaData temp;
	bool editing = false;

	void apply();
	void update_ui();
	void fill_systems_list();

	void add_new_system();
};

