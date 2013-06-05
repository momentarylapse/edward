/*
 * ModelMaterialDialog.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef MODELMATERIALDIALOG_H_
#define MODELMATERIALDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Stuff/Observer.h"

class ModelMaterialDialog: public HuiWindow, public Observer
{
public:
	ModelMaterialDialog(HuiWindow *_parent, bool _allow_parent, DataModel *_data);
	virtual ~ModelMaterialDialog();

	void LoadData();
	void ApplyData();

	void OnOk();
	void OnClose();
	void OnTransparencyMode();
	void OnMatAddTextureLevel();
	void OnMatTextures();
	void OnMatTexturesSelect();
	void OnMatDeleteTextureLevel();
	void OnMatEmptyTextureLevel();
	void OnDefaultMaterial();
	void OnFindMaterial();
	void OnEditMaterial();
	void OnDefaultColors();

	void FillTextureList();

	void OnUpdate(Observable *o);

private:
	DataModel *data;
	int index;
	ModelMaterial TempMaterial;
};

#endif /* MODELMATERIALDIALOG_H_ */
