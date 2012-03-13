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

class ModelMaterialDialog: public CHuiWindow
{
public:
	ModelMaterialDialog(CHuiWindow *_parent, bool _allow_parent, DataModel *_data);
	virtual ~ModelMaterialDialog();

	void LoadData();
	void ApplyData();

	void OnOk();
	void OnClose();
	void OnTransparencyMode();
	void OnDefaultTransparency();
	void OnMatAddTextureLevel();
	void OnMatTextures();
	void OnMatDeleteTextureLevel();
	void OnMatEmptyTextureLevel();
	void OnDefaultMaterial();
	void OnFindMaterial();
	void OnEditMaterial();
	void OnDefaultColors();

	void FillTextureList();

private:
	DataModel *data;
	ModeModelMaterial *mat;
	ModeModelMaterial TempMaterial;
};

#endif /* MODELMATERIALDIALOG_H_ */
