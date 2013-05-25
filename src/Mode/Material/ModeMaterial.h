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

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnCommand(const string &id);
	virtual void OnUpdate(Observable *o);

	virtual void OnDraw();
	virtual void OnDrawWin(int win);

	virtual void OnUpdateMenu();


	virtual void New();
	virtual bool Open();
	virtual bool Save();
	virtual bool SaveAs();

	void ExecuteAppearanceDialog();
	void ExecutePhysicsDialog();

	virtual bool OptimizeView();

	DataMaterial *data;
	virtual Data *GetData(){	return data;	}

	int MaterialVB[MATERIAL_MAX_TEXTURES];
	MaterialPropertiesDialog *AppearanceDialog;

	string shape_type;
	bool shape_smooth;

	void SetShapeType(const string &type);
	void SetShapeSmooth(bool smooth);
	void UpdateShape();
};

extern ModeMaterial* mode_material;

#endif /* MODEMATERIAL_H_ */
