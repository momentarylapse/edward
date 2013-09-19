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
#include "../../../Stuff/EmbeddedDialog.h"
#include "../../../Stuff/Observer.h"

class MaterialPropertiesDialog: public EmbeddedDialog, public Observer
{
public:
	MaterialPropertiesDialog(HuiWindow *_parent, DataMaterial *_data);
	virtual ~MaterialPropertiesDialog();

	void LoadData();
	void ApplyData();
	void ApplyDataDelayed();
	void ApplyPhysData();
	void ApplyPhysDataDelayed();

	void OnTextures();
	void OnTexturesSelect();
	void OnAddTextureLevel();
	void OnDeleteTextureLevel();
	void OnEmptyTextureLevel();
	void OnTransparencyMode();
	void OnReflectionMode();
	void OnReflectionTextures();
	void OnFindShader();

	void RefillReflTexView();
	void FillTextureList();

	void OnUpdate(Observable *o);


private:
	DataMaterial *data;
	DataMaterial::AppearanceData temp;
	DataMaterial::PhysicsData temp_phys;
	int apply_queue_depth;
	int apply_phys_queue_depth;
};

#endif /* MATERIALPROPERTIESDIALOG_H_ */
