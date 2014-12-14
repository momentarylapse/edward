/*
 * ModelMaterialSelectionDialog.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODELMATERIALSELECTIONDIALOG_H_
#define MODELMATERIALSELECTIONDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Stuff/Observer.h"
class DataModel;

class ModelMaterialSelectionDialog: public HuiWindow, public Observer
{
public:
	ModelMaterialSelectionDialog(HuiWindow *_parent, bool _allow_parent, DataModel *_data);
	virtual ~ModelMaterialSelectionDialog();

	void OnClose();
	void OnMaterialList();
	void OnMaterialListSelect();
	void OnMaterialAddNew();
	void OnMaterialAdd();
	void OnMaterialEdit();

	void onUpdate(Observable *o, const string &message);

	void FillMaterialList();

	void PutAnswer(int *_answer);
private:
	int *answer;
	DataModel *data;
};

#endif /* MODELMATERIALSELECTIONDIALOG_H_ */
