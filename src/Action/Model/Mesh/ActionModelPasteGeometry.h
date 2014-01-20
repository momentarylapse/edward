/*
 * ActionModelPasteGeometry.h
 *
 *  Created on: 07.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELPASTEGEOMETRY_H_
#define ACTIONMODELPASTEGEOMETRY_H_

#include "../../ActionGroup.h"
class DataModel;
class Geometry;

class ActionModelPasteGeometry : public ActionGroup
{
public:
	ActionModelPasteGeometry(Geometry &geo, int material);
	virtual ~ActionModelPasteGeometry(){}
	string name(){	return "ModelPasteGeometry";	}

	void *compose(Data *d);
private:
	Geometry &geo;
	int default_material;
};

#endif /* ACTIONMODELPASTEGEOMETRY_H_ */
