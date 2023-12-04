/*
 * ActionModelPasteGeometry.h
 *
 *  Created on: 07.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELPASTEGEOMETRY_H_
#define ACTIONMODELPASTEGEOMETRY_H_

#include "../../ActionGroup.h"
#include "../../../data/model/geometry/Geometry.h"
class DataModel;
class Geometry;

class ActionModelPasteGeometry : public Action {
public:
	ActionModelPasteGeometry(const Geometry &geo, int material);
	string name() override { return "ModelPasteGeometry"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	Geometry geo;
	int default_material;
	int num_edges_before;
};

#endif /* ACTIONMODELPASTEGEOMETRY_H_ */
