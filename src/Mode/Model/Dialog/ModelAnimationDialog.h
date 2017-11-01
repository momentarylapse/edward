/*
 * ModelAnimationDialog.h
 *
 *  Created on: 13.08.2012
 *      Author: michi
 */

#ifndef MODELANIMATIONDIALOG_H_
#define MODELANIMATIONDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Stuff/Observer.h"
class DataModel;

class ModelAnimationDialog: public hui::Panel, public Observer
{
public:
	ModelAnimationDialog(DataModel *_data);
	virtual ~ModelAnimationDialog();

	void loadData();
	void applyData();

	void fillAnimation();
	void onAnimationList();
	void onAnimationListSelect();
	void onAddAnimation();
	void onCopyAnimation();
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

	int getFirstFreeIndex();

private:
	DataModel *data;
};

#endif /* MODELANIMATIONDIALOG_H_ */
