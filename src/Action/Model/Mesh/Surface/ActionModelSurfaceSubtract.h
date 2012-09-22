/*
 * ActionModelSurfaceSubtract.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACESUBTRACT_H_
#define ACTIONMODELSURFACESUBTRACT_H_

#include "../../../ActionGroup.h"
class DataModel;
class ModelTriangle;
class ModelSurface;
class sCol;

class ActionModelSurfaceSubtract : public ActionGroup
{
public:
	ActionModelSurfaceSubtract(DataModel *m);
	virtual ~ActionModelSurfaceSubtract();
	string name(){	return "ModelSurfaceSubtract";	}

private:

	Array<sCol> t_col;

	bool CollideTriangles(DataModel *m, ModelTriangle *t1, ModelTriangle *t2, int t2_index);
	bool CollideTriangleSurface(DataModel *m, ModelTriangle *t, ModelSurface *s);
	bool TriangleInsideSurface(DataModel *m, ModelTriangle *t, ModelSurface *s);
	bool sort_t_col(ModelSurface *s, Array<sCol> &c2);
	void sort_and_join_contours(DataModel *m, ModelTriangle *t, ModelSurface *b, Array<Array<sCol> > &c, bool inverse);
	void TriangleSubtract(DataModel *m, ModelSurface *&a, ModelTriangle *t, ModelSurface *&b, bool inverse);
	void SurfaceSubtractUnary(DataModel *m, ModelSurface *&a, ModelSurface *&b, bool inverse);
	void SurfaceSubtract(DataModel *m, ModelSurface *&a, ModelSurface *&b);
};

#endif /* ACTIONMODELSURFACESUBTRACT_H_ */
