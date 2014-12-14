/*
 * ModelAnimationDialog.h
 *
 *  Created on: 13.08.2012
 *      Author: michi
 */

#ifndef MODELANIMATIONDIALOG_H_
#define MODELANIMATIONDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Stuff/EmbeddedDialog.h"
#include "../../../Stuff/Observer.h"
class DataModel;

class ModelAnimationDialog: public EmbeddedDialog, public Observer
{
public:
	ModelAnimationDialog(HuiWindow *_parent, DataModel *_data);
	virtual ~ModelAnimationDialog();

	void LoadData();
	void ApplyData();

	void FillAnimation();
	void OnAnimationList();
	void OnAnimationListSelect();
	void OnAddAnimation();
	void OnDeleteAnimation();
	void OnFrame();
	void OnAddFrame();
	void OnDeleteFrame();
	void OnName();
	void OnFpsConst();
	void OnFpsFactor();
	void OnSpeed();
	void OnParameter();
	void OnSimulationPlay();
	void OnSimulationStop();
	void OnClose();

	void onUpdate(Observable *o, const string &message);

	int GetSelectedAnimation();

private:
	DataModel *data;
};

#endif /* MODELANIMATIONDIALOG_H_ */
