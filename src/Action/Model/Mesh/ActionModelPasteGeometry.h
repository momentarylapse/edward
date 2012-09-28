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
class ModelGeometry;

class ActionModelPasteGeometry : public ActionGroup
{
public:
	ActionModelPasteGeometry(ModelGeometry &_geo);
	virtual ~ActionModelPasteGeometry(){}
	string name(){	return "ModelPasteGeometry";	}

	void *compose(Data *d);
private:
	ModelGeometry &geo;
};

#endif /* ACTIONMODELPASTEGEOMETRY_H_ */
