//
// Created by Michael Ankele on 2025-02-10.
//

#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <lib/xhui/Dialog.h>
#include "../data/DataWorld.h"


class PropertiesDialog : public xhui::Dialog {
public:
	explicit PropertiesDialog(xhui::Panel* parent, DataWorld* data);
	DataWorld* data;

	DataWorld::MetaData temp;

	void apply();
	void fill();
};



#endif //PROPERTIESDIALOG_H
