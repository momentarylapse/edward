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

class SelectionPropertiesDialog: public HuiWindow, public Observer
{
public:
	SelectionPropertiesDialog(HuiWindow *_parent, bool _allow_parent, DataWorld *_data, int *_selected_type, int *_selected_index);
	virtual ~SelectionPropertiesDialog();

	void LoadData();

	void FillList();
	void OnPropertyList();
	void OnClose();

	void OnUpdate(Observable *o);

private:
	DataWorld *data;
	Array<int> type, index;
	int *selected_type;
	int *selected_index;
};

#endif /* SELECTIONPROPERTIESDIALOG_H_ */
