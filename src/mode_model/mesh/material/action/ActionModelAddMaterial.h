/*
 * ActionModelAddMaterial.h
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDMATERIAL_H_
#define ACTIONMODELADDMATERIAL_H_

#include <action/Action.h>
#include <lib/base/pointer.h>

class ModelMaterial;

class ActionModelAddMaterial : public Action {
public:
	explicit ActionModelAddMaterial(xfer<ModelMaterial> material);
	string name() override { return "ModelAddMaterial"; }

	void* execute(Data* d) override;
	void undo(Data* d) override;
private:
	owned<ModelMaterial> material;
};

#endif /* ACTIONMODELADDMATERIAL_H_ */
