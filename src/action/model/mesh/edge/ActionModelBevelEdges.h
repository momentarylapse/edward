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
class ModelMesh;
class ModelPolygon;
class VertexToCome;
class PolygonRelink;

class ActionModelBevelEdges: public ActionGroup {
public:
	ActionModelBevelEdges(float _length, int material);
	string name(){	return "ModelBevelEdges";	}

	void *compose(Data *d);

	void bevelSurface(ModelMesh *m);
	void build_vertices(Array<VertexToCome> &vv, DataModel *m);
	void do_poly_relink(ModelPolygon &p, PolygonRelink &r, int i, DataModel *m);
private:
	float length;
	int material;
};

#endif /* ACTIONMODELBEVELEDGES_H_ */
