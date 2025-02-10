//
// Created by Michael Ankele on 2025-02-10.
//

#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <lib/xhui/Dialog.h>


class DataWorld;

class PropertiesDialog : public xhui::Dialog {
public:
	explicit PropertiesDialog(xhui::Panel* parent, DataWorld* data);
	DataWorld* data;
};



#endif //PROPERTIESDIALOG_H
