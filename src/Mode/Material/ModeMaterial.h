/*
 * ModeMaterial.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef MODEMATERIAL_H_
#define MODEMATERIAL_H_

#include "../Mode.h"
#include "../../Data/Material/DataMaterial.h"

class Mode;
class DataMaterial;
class MaterialPropertiesDialog;

class ModeMaterial: public Mode
{
public:
	ModeMaterial();
	virtual ~ModeMaterial();

	void OnStart();
	void OnEnd();

	void OnCommand(const string &id);
	void OnUpdate(Observable *o);

	void OnDraw();
	void OnDrawWin(int win);


	void New();
	bool Open();
	bool Save();
	bool SaveAs();

	void ExecuteAppearanceDialog();
	void ExecutePhysicsDialog();

	bool OptimizeView();

	DataMaterial *data;
	Data *GetData(){	return data;	}

	int MaterialVB[MATERIAL_MAX_TEXTURES];
	MaterialPropertiesDialog *AppearanceDialog;
};

extern ModeMaterial* mode_material;

#endif /* MODEMATERIAL_H_ */
