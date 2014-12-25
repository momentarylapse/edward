/*
 * ModelNewAnimationDialog.h
 *
 *  Created on: 14.08.2012
 *      Author: michi
 */

#ifndef MODELNEWANIMATIONDIALOG_H_
#define MODELNEWANIMATIONDIALOG_H_

#include "../../../lib/hui/hui.h"
class DataModel;

class ModelNewAnimationDialog: public HuiWindow
{
public:
	ModelNewAnimationDialog(HuiWindow *_parent, bool _allow_parent, DataModel *_data, int index, int type);
	virtual ~ModelNewAnimationDialog();

	void onClose();
	void onOk();

private:
	DataModel *data;
};

#endif /* MODELNEWANIMATIONDIALOG_H_ */
