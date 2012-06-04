/*
 * ModelMaterialSelectionDialog.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODELMATERIALSELECTIONDIALOG_H_
#define MODELMATERIALSELECTIONDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/Model/DataModel.h"

class ModelMaterialSelectionDialog: public CHuiWindow
{
public:
	ModelMaterialSelectionDialog(CHuiWindow *_parent, bool _allow_parent, DataModel *_data);
	virtual ~ModelMaterialSelectionDialog();

	void OnClose();
	void OnMaterialList();
	void OnMaterialListCheck();

	void FillMaterialList();

	void PutAnswer(int *_answer);
private:
	int *answer;
	DataModel *data;
};

#endif /* MODELMATERIALSELECTIONDIALOG_H_ */
