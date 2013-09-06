/*
 * ActionModelSurfaceVolumeSubtract.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACEVOLUMESUBTRACT_H_
#define ACTIONMODELSURFACEVOLUMESUBTRACT_H_

#include "../../../ActionGroup.h"
#include "../../../../lib/math/math.h"
class DataModel;
class ModelPolygon;
class ModelSurface;
class ModelGeometry;

class ActionModelSurfaceVolumeSubtract : public ActionGroup
{
public:
	ActionModelSurfaceVolumeSubtract();
	virtual ~ActionModelSurfaceVolumeSubtract(){};
	string name(){	return "ModelSurfaceVolumeSubtract";	}

	void *compose(Data *d);

private:
	void SurfaceSubtract(DataModel *m, ModelSurface *a, int ai, ModelSurface *b, Array<ModelGeometry> &geos);
};

#endif /* ACTIONMODELSURFACEVOLUMESUBTRACT_H_ */
