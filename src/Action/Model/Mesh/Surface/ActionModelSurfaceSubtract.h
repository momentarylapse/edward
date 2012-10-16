/*
 * ActionModelSurfaceSubtract.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACESUBTRACT_H_
#define ACTIONMODELSURFACESUBTRACT_H_

#include "../../../ActionGroup.h"
#include "../../../../lib/types/types.h"
class DataModel;
class ModelPolygon;
class ModelSurface;

class ActionModelSurfaceSubtract : public ActionGroup
{
public:
	ActionModelSurfaceSubtract();
	virtual ~ActionModelSurfaceSubtract(){};
	string name(){	return "ModelSurfaceSubtract";	}

	void *compose(Data *d);

private:

	class sCol
	{
	public:
		sCol(){}
		sCol(const vector &_p, bool _own_edge, int _polygon, int _edge){
			p = _p;
			own_edge = _own_edge;
			polygon = _polygon;
			edge = _edge;
		}
		vector p;
		bool own_edge;
		int polygon, edge;
	};

	Array<sCol> t_col;

	bool CollidePolygons(DataModel *m, ModelPolygon *t1, ModelPolygon *t2, int t2_index);
	bool CollidePolygonSurface(DataModel *m, ModelPolygon *t, ModelSurface *s, int t_index);
	bool PolygonInsideSurface(DataModel *m, ModelPolygon *t, ModelSurface *s);
	bool sort_t_col(ModelSurface *s, Array<sCol> &c2);
	void sort_and_join_contours(DataModel *m, ModelPolygon *t, ModelSurface *b, Array<Array<sCol> > &c, bool inverse);
	void PolygonSubtract(DataModel *m, ModelSurface *&a, ModelPolygon *t, ModelSurface *&b, bool inverse);
	void SurfaceSubtractUnary(DataModel *m, ModelSurface *&a, ModelSurface *&b, bool inverse);
	void SurfaceSubtract(DataModel *m, ModelSurface *&a, ModelSurface *&b);
};

#endif /* ACTIONMODELSURFACESUBTRACT_H_ */
