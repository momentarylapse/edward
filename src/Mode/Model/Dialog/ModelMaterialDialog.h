/*
 * ModelMaterialDialog.h
 *
 *  Created on: 19.09.2013
 *      Author: michi
 */

#ifndef MODELMATERIALDIALOG_H_
#define MODELMATERIALDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Stuff/EmbeddedDialog.h"
#include "../../../Stuff/Observer.h"
#include "../../../Data/Model/ModelMaterial.h"
class DataModel;

class ModelMaterialDialog: public EmbeddedDialog, public Observer
{
public:
	ModelMaterialDialog(HuiWindow *_parent, DataModel *_data);
	virtual ~ModelMaterialDialog();

	void LoadData();
	void ApplyData();
	void ApplyDataDelayed();
	void FillMaterialList();

	void OnMaterialList();
	void OnMaterialListSelect();
	void OnAddNewMaterial();
	void OnAddMaterial();
	void OnDeleteMaterial();
	void OnApplyMaterial();

	void OnDefaultColors();

	void OnTextures();
	void OnTexturesSelect();
	void OnAddTextureLevel();
	void OnDeleteTextureLevel();
	void OnEmptyTextureLevel();
	void OnTransparencyMode();

	void FillTextureList();

	void onUpdate(Observable *o, const string &message);

private:
	DataModel *data;
	ModelMaterial temp;
	int apply_queue_depth;
};

#endif /* MODELMATERIALDIALOG_H_ */
