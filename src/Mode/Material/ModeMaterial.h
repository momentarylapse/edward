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
#include "Dialog/MaterialPropertiesDialog.h"

class Mode;
class DataMaterial;
class MaterialPropertiesDialog;

class ModeMaterial: public Mode
{
public:
	ModeMaterial();
	virtual ~ModeMaterial();

	void Start();
	void End();

	void OnMouseMove();
	void OnLeftButtonDown();
	void OnLeftButtonUp();
	void OnMiddleButtonDown();
	void OnMiddleButtonUp();
	void OnRightButtonDown();
	void OnRightButtonUp();
	void OnKeyDown();
	void OnKeyUp();
	void OnCommand(const string &id);
	void OnUpdate(Observable *o);
	void OnUpdateMenu();

	void Draw();
	void DrawWin(int win, irect dest);


	void New();
	bool Open();
	bool Save();
	bool SaveAs();

	void ExecuteAppearanceDialog();

	DataMaterial *data;

	int MaterialVB[MATERIAL_MAX_TEXTURE_LEVELS];
	MaterialPropertiesDialog *AppearanceDialog;
};

extern ModeMaterial* mode_material;

#endif /* MODEMATERIAL_H_ */
