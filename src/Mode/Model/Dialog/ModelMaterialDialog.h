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

class ModelMaterialDialog: public hui::Panel, public Observer
{
public:
	ModelMaterialDialog(DataModel *_data);
	virtual ~ModelMaterialDialog();

	void loadData();
	void applyData();
	void applyDataDelayed();
	void fillMaterialList();

	void onMaterialList();
	void onMaterialListSelect();
	void onAddNewMaterial();
	void onAddMaterial();
	void onDeleteMaterial();
	void onApplyMaterial();

	void onDefaultColors();

	void onTextures();
	void onTexturesSelect();
	void onAddTextureLevel();
	void onDeleteTextureLevel();
	void onEmptyTextureLevel();
	void onTransparencyMode();

	void fillTextureList();

	void onUpdate(Observable *o, const string &message);

private:
	DataModel *data;
	ModelMaterial temp;
	int apply_queue_depth;
};

#endif /* MODELMATERIALDIALOG_H_ */
