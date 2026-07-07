/*
 * ActionModelSetMaterial.h
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSETMATERIAL_H_
#define ACTIONMODELSETMATERIAL_H_

#include <lib/history/Action.h>
#include <view/Selection.h>
class DataModel;

class ActionModelSetMaterial: public history::Action {
public:
	ActionModelSetMaterial(DataModel *m, const Selection& sel, int _material);
	string name() const override {	return "ModelSetMaterial";	}

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	int material;
	Array<int> triangle;
	Array<int> old_material;
};

#endif /* ACTIONMODELSETMATERIAL_H_ */
