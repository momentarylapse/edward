/*
 * ActionModelSurfaceSubtract.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACESUBTRACT_H_
#define ACTIONMODELSURFACESUBTRACT_H_

#include "../../../ActionGroup.h"
#include "../../../../lib/math/math.h"
class DataModel;
class ModelPolygon;
class ModelSurface;
class ModelGeometry;

class ActionModelSurfaceSubtract : public ActionGroup
{
public:
	ActionModelSurfaceSubtract();
	virtual ~ActionModelSurfaceSubtract(){};
	string name(){	return "ModelSurfaceSubtract";	}

	void *compose(Data *d);

private:
	void SurfaceSubtract(DataModel *m, ModelSurface *a, int ai, ModelSurface *b, Array<ModelGeometry> &geos);
};

#endif /* ACTIONMODELSURFACESUBTRACT_H_ */
