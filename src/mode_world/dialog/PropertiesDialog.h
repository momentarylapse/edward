//
// Created by Michael Ankele on 2025-02-10.
//

#pragma once

#include <lib/xhui/Dialog.h>
#include <lib/pattern/Observable.h>
#include "../data/DataWorld.h"


class PropertiesDialog : public obs::Node<xhui::Panel> {
public:
	explicit PropertiesDialog(DataWorld* data);
	DataWorld* data;

	DataWorld::MetaData temp;

	void apply();
	void fill();

	void add_new_system();
};


