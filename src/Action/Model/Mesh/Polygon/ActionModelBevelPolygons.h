/*
 * ActionModelBevelPolygons.h
 *
 *  Created on: 06.01.2013
 *      Author: michi
 */

#ifndef ACTIONMODELBEVELPOLYGONS_H_
#define ACTIONMODELBEVELPOLYGONS_H_

#include "../../../ActionGroup.h"
class DataModel;
class ModelSurface;
class ModelPolygon;
class VertexToCome;
class PolygonRelink;

class ActionModelBevelPolygons: public ActionGroup
{
public:
	ActionModelBevelPolygons(float _length);
	virtual ~ActionModelBevelPolygons(){}
	string name(){	return "ModelBevelPolygons";	}

	void *compose(Data *d);

	void BevelSurface(DataModel *m, ModelSurface *s, int surface);
	void build_vertices(Array<VertexToCome> &vv, DataModel *m);
	void do_poly_relink(ModelPolygon &p, PolygonRelink &r, int i, int surface, DataModel *m);
private:
	float length;
};

#endif /* ACTIONMODELBEVELPOLYGONS_H_ */
