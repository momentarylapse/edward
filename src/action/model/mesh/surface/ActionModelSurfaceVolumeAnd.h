/*
 * ActionModelSurfaceVolumeAnd.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACEVOLUMEAND_H_
#define ACTIONMODELSURFACEVOLUMEAND_H_

#include "../../../ActionGroup.h"
#include "../../../../lib/math/math.h"
class DataModel;
class ModelPolygon;
class ModelSurface;
class Geometry;

class ActionModelSurfaceVolumeAnd : public ActionGroup
{
public:
	ActionModelSurfaceVolumeAnd();
	virtual ~ActionModelSurfaceVolumeAnd(){};
	string name(){	return "ModelSurfaceVolumeAnd";	}

	void *compose(Data *d);

private:
	void SurfaceAnd(DataModel *m, ModelSurface *a, int ai, ModelSurface *b, Array<Geometry> &geos);
};

#endif /* ACTIONMODELSURFACEVOLUMEAND_H_ */
