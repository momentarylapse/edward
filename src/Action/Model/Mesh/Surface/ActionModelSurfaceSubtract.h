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
		enum{
			TYPE_OWN_EDGE_IN,
			TYPE_OWN_EDGE_OUT,
			TYPE_OTHER_EDGE,
			TYPE_OLD_VERTEX
		};
		sCol(){}
		sCol(const vector &_p, int _side){
			p = _p;
			type = TYPE_OLD_VERTEX;
			polygon = -1;
			edge = -1;
			side = _side;
		}
		sCol(const vector &_p, int _type, int _polygon, int _edge, int _side){
			p = _p;
			type = _type;
			polygon = _polygon;
			edge = _edge;
			side = _side;
		}
		float get_f(DataModel *m, ModelPolygon *t);
		vector p;
		int type;
		int polygon, edge, side;
	};

	Array<sCol> t_col;

	bool CollidePolygons(DataModel *m, ModelPolygon *t1, ModelPolygon *t2, int t2_index);
	bool CollidePolygonSurface(DataModel *m, ModelPolygon *t, ModelSurface *s, int t_index);
	bool PolygonInsideSurface(DataModel *m, ModelPolygon *t, ModelSurface *s);
	void sort_t_col(ModelSurface *s, Array<sCol> &c2);
	void sort_and_join_contours(DataModel *m, ModelPolygon *t, ModelSurface *b, Array<Array<sCol> > &c, bool inverse);
	void PolygonSubtract(DataModel *m, ModelSurface *&a, ModelPolygon *t, int t_index, ModelSurface *&b, bool inverse);
	void SurfaceSubtractUnary(DataModel *m, ModelSurface *&a, ModelSurface *&b, bool inverse);
	void SurfaceSubtract(DataModel *m, ModelSurface *a, ModelSurface *b);
};

#endif /* ACTIONMODELSURFACESUBTRACT_H_ */
