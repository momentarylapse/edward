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

	void loadData();
	void applyData();

	void fillAnimation();
	void onAnimationList();
	void onAnimationListSelect();
	void onAddAnimation();
	void onDeleteAnimation();
	void onFrame();
	void onAddFrame();
	void onDeleteFrame();
	void onName();
	void onFpsConst();
	void onFpsFactor();
	void onSpeed();
	void onParameter();
	void onSimulationPlay();
	void onSimulationStop();
	void onClose();

	void onUpdate(Observable *o, const string &message);

	int getSelectedAnimation();

private:
	DataModel *data;
};

#endif /* MODELANIMATIONDIALOG_H_ */
