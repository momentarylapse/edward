/*
 * MaterialPropertiesDialog.h
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#ifndef MATERIALPROPERTIESDIALOG_H_
#define MATERIALPROPERTIESDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/Material/DataMaterial.h"

class MaterialPropertiesDialog: public CHuiWindow
{
public:
	MaterialPropertiesDialog(CHuiWindow *_parent, bool _allow_parent, DataMaterial *_data);
	virtual ~MaterialPropertiesDialog();

	void LoadData();
	void ApplyData();

	void OnTextures();
	void OnTexturesSelect();
	void OnAddTextureLevel();
	void OnDeleteTextureLevel();
	void OnEmptyTextureLevel();
	void OnTransparencyMode();
	void OnReflection();
	void OnReflectionTextures();
	void OnFindEffect();
	void OnOk();
	void OnClose();

	void RefillReflTexView();
	void FillTextureList();


private:
	DataMaterial *data;
	int TempNumTextureLevels;
	string TempTextureFile[32];
	string ReflTex[6];
};

#endif /* MATERIALPROPERTIESDIALOG_H_ */
