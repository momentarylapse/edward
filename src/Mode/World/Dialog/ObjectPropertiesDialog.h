/*
 * ObjectPropertiesDialog.h
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#ifndef OBJECTPROPERTIESDIALOG_H_
#define OBJECTPROPERTIESDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Stuff/Observer.h"

class ObjectPropertiesDialog: public HuiWindow, public Observer
{
public:
	ObjectPropertiesDialog(HuiWindow *_parent, bool _allow_parent, DataWorld *_data, int _index);
	virtual ~ObjectPropertiesDialog();

	void LoadData();
	void ApplyData();

	void OnFindObject();
	void OnOk();
	void OnClose();

	void OnUpdate(Observable *o);

private:
	DataWorld *data;
	int index;

	WorldObject temp;
};

#endif /* OBJECTPROPERTIESDIALOG_H_ */
