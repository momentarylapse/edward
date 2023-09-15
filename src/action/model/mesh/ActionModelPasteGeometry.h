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

class ActionModelPasteGeometry : public ActionGroup {
public:
	ActionModelPasteGeometry(const Geometry &geo, int material);
	string name() override { return "ModelPasteGeometry"; }

	void *compose(Data *d) override;
private:
	Geometry geo;
	int default_material;
};

#endif /* ACTIONMODELPASTEGEOMETRY_H_ */
