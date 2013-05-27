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

class MaterialPropertiesDialog;
class ModelGeometry;

class ModeMaterial: public Mode<DataMaterial>
{
public:
	ModeMaterial();
	virtual ~ModeMaterial();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnCommand(const string &id);
	virtual void OnUpdate(Observable *o);

	virtual void OnDraw();
	virtual void OnDrawWin(MultiViewWindow *win);

	virtual void OnUpdateMenu();


	virtual void New();
	virtual bool Open();
	virtual bool Save();
	virtual bool SaveAs();

	void ExecuteAppearanceDialog();
	void ExecutePhysicsDialog();

	virtual bool OptimizeView();

	int MaterialVB[MATERIAL_MAX_TEXTURES];
	MaterialPropertiesDialog *AppearanceDialog;

	string shape_type;
	bool shape_smooth;
	ModelGeometry *geo;

	void SetShapeType(const string &type);
	void SetShapeSmooth(bool smooth);
	void UpdateShape();
};

extern ModeMaterial* mode_material;

#endif /* MODEMATERIAL_H_ */
