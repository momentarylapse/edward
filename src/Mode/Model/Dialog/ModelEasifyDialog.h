/*
 * ModelEasifyDialog.h
 *
 *  Created on: 26.08.2012
 *      Author: michi
 */

#ifndef MODELEASIFYDIALOG_H_
#define MODELEASIFYDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Stuff/Observer.h"
class DataModel;

class ModelEasifyDialog: public CHuiWindow, public Observer
{
public:
	ModelEasifyDialog(CHuiWindow *_parent, bool _allow_parent, DataModel *_data);
	virtual ~ModelEasifyDialog();

	void LoadData();

	void OnQualityFactor();
	void OnClose();
	void OnOk();

	void OnUpdate(Observable *o);

private:
	DataModel *data;
	float factor;
};

#endif /* MODELEASIFYDIALOG_H_ */
