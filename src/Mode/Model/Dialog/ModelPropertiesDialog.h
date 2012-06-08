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
	void OnGenerateDistsAuto();
	void OnGenerateDists();
	void OnGenerateSkin2();
	void OnGenerateSkin3();
	void OnMaterialList();
	void OnMaterialListCheck();
	void OnMaterialListSelect();
	void OnAddMaterial();
	void OnDeleteMaterial();
	void OnPhysicsPassive();
	void OnGenerateTensorAuto();
	void OnNumItems();
	void OnModelInventary();
	void OnDeleteItem();
	void OnMaxScriptVars();
	void OnScriptFind();
	void OnModelScriptVarTemplate();
	void OnOk();
	void OnClose();

	void OnUpdate(Observable *o);

private:
	DataModel *data;

	Array<string> temp_item;
	Array<float> temp_script_var;
};

#endif /* MODELPROPERTIESDIALOG_H_ */
