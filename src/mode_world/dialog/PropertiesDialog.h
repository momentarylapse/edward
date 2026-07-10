//
// Created by Michael Ankele on 2025-02-10.
//

#pragma once

#include <lib/xhui/Dialog.h>
#include <lib/obs/Observable.h>
#include "../data/DataWorld.h"


class PropertiesDialog : public obs::Node<xhui::Panel> {
public:
	explicit PropertiesDialog(DataWorld* data);
	DataWorld* data;

	DataWorld::MetaData temp;
	bool editing = false;

	void apply();
	void update_ui();
};


