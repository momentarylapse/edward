/*
 * ActionModelSetMaterial.h
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSETMATERIAL_H_
#define ACTIONMODELSETMATERIAL_H_

#include <action/Action.h>
class DataModel;

class ActionModelSetMaterial: public Action {
public:
	ActionModelSetMaterial(DataModel *m, const Data::Selection& sel, int _material);
	string name() override {	return "ModelSetMaterial";	}

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int material;
	Array<int> triangle;
	Array<int> old_material;
};

#endif /* ACTIONMODELSETMATERIAL_H_ */
