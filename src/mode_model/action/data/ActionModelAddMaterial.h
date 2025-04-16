/*
 * ActionModelAddMaterial.h
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDMATERIAL_H_
#define ACTIONMODELADDMATERIAL_H_

#include <action/Action.h>

class ActionModelAddMaterial : public Action {
public:
	explicit ActionModelAddMaterial(const Path &filename);
	string name() override { return "ModelAddMaterial"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	Path filename;
};

#endif /* ACTIONMODELADDMATERIAL_H_ */
