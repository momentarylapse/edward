/*
 * ActionModelDeleteMaterial.h
 *
 *  Created on: Dec 16, 2020
 *      Author: michi
 */

#ifndef SRC_ACTION_MODEL_DATA_ACTIONMODELDELETEMATERIAL_H_
#define SRC_ACTION_MODEL_DATA_ACTIONMODELDELETEMATERIAL_H_

#include <action/Action.h>

class ModelMaterial;

class ActionModelDeleteMaterial : public Action {
public:
	explicit ActionModelDeleteMaterial(int index);
	string name() override { return "ModelDeleteMaterial"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	int index;
	ModelMaterial *mat;
};

#endif /* SRC_ACTION_MODEL_DATA_ACTIONMODELDELETEMATERIAL_H_ */
