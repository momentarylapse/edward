/*
 * MaterialPhysicsDialog.h
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#ifndef MATERIALPHYSICSDIALOG_H_
#define MATERIALPHYSICSDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/Material/DataMaterial.h"

class MaterialPhysicsDialog: public CHuiWindow
{
public:
	MaterialPhysicsDialog(CHuiWindow *_parent, bool _allow_parent, DataMaterial *_data);
	virtual ~MaterialPhysicsDialog();

	void LoadData();
	void ApplyData();

	void OnOk();
	void OnClose();


private:
	DataMaterial *data;
	DataMaterial::PhysicsData temp;
};

#endif /* MATERIALPHYSICSDIALOG_H_ */
