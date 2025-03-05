//
// Created by Michael Ankele on 2025-03-05.
//

#ifndef MODELPROPERTIESDIALOG_H
#define MODELPROPERTIESDIALOG_H

#include <lib/xhui/Dialog.h>

class DataModel;

class ModelPropertiesDialog : public xhui::Dialog {
public:
	ModelPropertiesDialog(xhui::Panel* parent, DataModel* data);
	DataModel* data;

	void apply();
	void fill();
};



#endif //MODELPROPERTIESDIALOG_H
