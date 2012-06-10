/*
 * ActionModelSurfaceSubtract.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACESUBTRACT_H_
#define ACTIONMODELSURFACESUBTRACT_H_

#include "../../../Action.h"
#include "../../../ActionGroup.h"
#include "../../../../lib/types/types.h"
#include "../../../../Data/Model/DataModel.h"

class ActionModelSurfaceSubtract : public ActionGroup
{
public:
	ActionModelSurfaceSubtract(DataModel *m);
	virtual ~ActionModelSurfaceSubtract();
	string name(){	return "ModelSurfaceSubtract";	}

private:

	struct sCol
	{
		sCol(){}
		sCol(const vector &_p, bool _own_edge, int _index, int _k){
			p = _p;
			own_edge = _own_edge;
			index = _index;
			k = _k;
		}
		vector p;
		bool own_edge;
		int index, k;
	};

	Array<sCol> t_col;

	bool CollideTriangles(DataModel *m, ModeModelTriangle *t1, ModeModelTriangle *t2, int t2_index);
	bool CollideTriangleSurface(DataModel *m, ModeModelTriangle *t, ModeModelSurface *s);
	bool TriangleInsideSurface(DataModel *m, ModeModelTriangle *t, ModeModelSurface *s);
	bool sort_t_col(ModeModelSurface *s, Array<sCol> &c2);
	void sort_and_join_contours(DataModel *m, ModeModelTriangle *t, ModeModelSurface *b, Array<Array<sCol> > &c, bool inverse);
	void TriangleSubtract(DataModel *m, ModeModelSurface *&a, ModeModelTriangle *t, ModeModelSurface *&b, bool inverse);
	void SurfaceSubtractUnary(DataModel *m, ModeModelSurface *&a, ModeModelSurface *&b, bool inverse);
	void SurfaceSubtract(DataModel *m, ModeModelSurface *&a, ModeModelSurface *&b);
};

#endif /* ACTIONMODELSURFACESUBTRACT_H_ */
