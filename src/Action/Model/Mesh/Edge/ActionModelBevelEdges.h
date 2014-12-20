/*
 * ActionModelBevelEdges.h
 *
 *  Created on: 06.01.2013
 *      Author: michi
 */

#ifndef ACTIONMODELBEVELEDGES_H_
#define ACTIONMODELBEVELEDGES_H_

#include "../../../ActionGroup.h"
class DataModel;
class ModelSurface;
class ModelPolygon;
class VertexToCome;
class PolygonRelink;

class ActionModelBevelEdges: public ActionGroup
{
public:
	ActionModelBevelEdges(float _length);
	string name(){	return "ModelBevelEdges";	}

	void *compose(Data *d);

	void bevelSurface(DataModel *m, ModelSurface *s, int surface);
	void build_vertices(Array<VertexToCome> &vv, DataModel *m);
	void do_poly_relink(ModelPolygon &p, PolygonRelink &r, int i, int surface, DataModel *m);
private:
	float length;
};

#endif /* ACTIONMODELBEVELEDGES_H_ */
