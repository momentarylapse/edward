/*
 * ModelPropertiesDialog.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef MODELPROPERTIESDIALOG_H_
#define MODELPROPERTIESDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Stuff/Observer.h"

class ModelPropertiesDialog: public CHuiWindow, public Observer
{
public:
	ModelPropertiesDialog(CHuiWindow *_parent, bool _allow_parent, DataModel *_data);
	virtual ~ModelPropertiesDialog();

	void LoadData();
	void ApplyData();

	void FillMaterialList();
	void RefillInventaryList();
	void RefillScriptVarList();
	void FillDetailList();
	void FillTensorList();
	void OnGenerateDistsAuto();
	void OnMaterialList();
	void OnMaterialListCheck();
	void OnMaterialListSelect();
	void OnAddNewMaterial();
	void OnAddMaterial();
	void OnDeleteMaterial();
	void OnPhysicsPassive();
	void OnGenerateTensorAuto();
	void OnTensorEdit();
	void OnNumItems();
	void OnModelInventary();
	void OnDeleteItem();
	void OnMaxScriptVars();
	void OnScriptVarEdit();
	void OnScriptFind();
	void OnModelScriptVarTemplate();
	void OnOk();
	void OnClose();

	void OnUpdate(Observable *o);

private:
	DataModel *data;

	DataModel::MetaData temp;
};

#endif /* MODELPROPERTIESDIALOG_H_ */
