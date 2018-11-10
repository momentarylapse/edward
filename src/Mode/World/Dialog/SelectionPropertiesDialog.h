/*
 * SelectionPropertiesDialog.h
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#ifndef SELECTIONPROPERTIESDIALOG_H_
#define SELECTIONPROPERTIESDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Stuff/Observer.h"

class SelectionPropertiesDialog: public hui::Dialog, public Observer
{
public:
	SelectionPropertiesDialog(hui::Window *_parent, bool _allow_parent, DataWorld *_data, int *_selected_type, int *_selected_index);
	virtual ~SelectionPropertiesDialog();

	void LoadData();

	void FillList();
	void OnPropertyList();
	void OnClose();

	void on_update(Observable *o, const string &message) override;

private:
	DataWorld *data;
	Array<int> type, index;
	int *selected_type;
	int *selected_index;
};

#endif /* SELECTIONPROPERTIESDIALOG_H_ */
