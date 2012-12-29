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
		sCol(const vector &_p, int _side);
		sCol(const vector &_p, int _type, int _polygon, int _edge, int _side);
		float get_f(DataModel *m, ModelPolygon *t);
		bool operator==(const sCol &other) const;
		vector p;
		int type;
		int polygon, edge, side;
		string str() const;
	};

	Array<sCol> col;

	bool CollidePolygons(DataModel *m, ModelPolygon *t1, ModelPolygon *t2, int t2_index);
	bool CollidePolygonSurface(DataModel *m, ModelPolygon *t, ModelSurface *s, int t_index);
	bool PolygonInsideSurface(DataModel *m, ModelPolygon *t, ModelSurface *s);
	void find_contours(DataModel *m, ModelPolygon *t, ModelSurface *s, Array<Array<sCol> > &c_out, bool inverse);
	bool find_contour_boundary(ModelSurface *s, Array<sCol> &c_in, Array<sCol> &c_out, bool inverse);
	bool find_contour_inside(DataModel *m, ModelPolygon *t, ModelSurface *s, Array<sCol> &c_in, Array<sCol> &c_out, bool inverse);
	float get_ang(Array<sCol> &c, int i, const vector &flat_n);
	bool vertex_in_tria(sCol &a, sCol &b, sCol &c, sCol &v, float &slope);
	void combine_contours(Array<Array<sCol> > &c, int ca, int ia, int cb, int ib);
	void triangulate_contours(DataModel *m, ModelPolygon *t, Array<Array<sCol> > &c);
	bool combine_polygons(Array<Array<sCol> > &c, int ia, int ib);
	void simplify_filling(Array<Array<sCol> > &c);
	void sort_and_join_contours(DataModel *m, ModelPolygon *t, ModelSurface *b, Array<Array<sCol> > &c, bool inverse);
	void PolygonSubtract(DataModel *m, ModelSurface *&a, ModelPolygon *t, int t_index, ModelSurface *&b, bool inverse);
	void SurfaceSubtractUnary(DataModel *m, ModelSurface *&a, ModelSurface *&b, bool inverse);
	void SurfaceSubtract(DataModel *m, ModelSurface *a, ModelSurface *b);
};

#endif /* ACTIONMODELSURFACESUBTRACT_H_ */
