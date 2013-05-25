/*
 * ModelGeometryCylinder.cpp
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#include "ModelGeometryCylinder.h"
#include "../DataModel.h"

#define _cyl_vert(i, j)         ( edges      * (i) +(j) % edges)
#define _cyl_svert(i, j)        sv[(edges + 1) * (i) +(j) % (edges + 1)]

ModelGeometryCylinder::ModelGeometryCylinder(Array<vector> &pos, Array<float> &radius, int rings, int edges, bool closed)
{
	Interpolator<float> inter_r(Interpolator<float>::TYPE_CUBIC_SPLINE_NOTANG);
	foreach(float r, radius)
		inter_r.add(r);

	// vertices (interpolated on path)
	Interpolator<vector> inter(Interpolator<vector>::TYPE_CUBIC_SPLINE_NOTANG);
	foreach(vector &p, pos)
		inter.add(p);

	BuildFromPath(inter, inter_r, rings, edges, closed);
}

ModelGeometryCylinder::ModelGeometryCylinder(Array<vector> &pos, float radius, int rings, int edges, bool closed)
{
	Interpolator<float> inter_r(Interpolator<float>::TYPE_CUBIC_SPLINE_NOTANG);
	inter_r.add(radius);
	inter_r.add(radius);

	// vertices (interpolated on path)
	Interpolator<vector> inter(Interpolator<vector>::TYPE_CUBIC_SPLINE_NOTANG);
	foreach(vector &p, pos)
		inter.add(p);

	BuildFromPath(inter, inter_r, rings, edges, closed);
}

ModelGeometryCylinder::ModelGeometryCylinder(vector &pos1, vector &pos2, float radius, int rings, int edges, bool closed)
{
	Interpolator<float> inter_r(Interpolator<float>::TYPE_CUBIC_SPLINE_NOTANG);
	inter_r.add(radius);
	inter_r.add(radius);

	// vertices (interpolated on path)
	Interpolator<vector> inter(Interpolator<vector>::TYPE_CUBIC_SPLINE_NOTANG);
	inter.add(pos1);
	inter.add(pos2);

	BuildFromPath(inter, inter_r, rings, edges, closed);
}

void ModelGeometryCylinder::BuildFromPath(Interpolator<vector> &inter, Interpolator<float> &inter_r, int rings, int edges, bool closed)
{
	Array<vector> sv;
	vector r_last = v_0;
	for (int i=0;i<=rings;i++){
		// interpolated point on path
		float t = (float)i / (float)rings;
		vector p0 = inter.get(t);
		vector dir = inter.get_tang(t);

		// moving frame
		vector u = r_last ^ dir;
		if (i == 0)
			u = dir.ortho();
		u.normalize();
		vector r = dir ^ u;
		r.normalize();
		r_last = r;

		// vertex ring
		float radius = inter_r.get(t);
		for (int j=0;j<=edges;j++){
			float w = pi*2*(float)j/(float)edges;
			vector p = p0+((float)sin(w)*u+(float)cos(w)*r)*radius;
			if (j < edges)
				AddVertex(p);
			sv.add(vector((float)j/(float)edges,t,0));
		}
	}

// the curved surface
	for (int i=0;i<rings;i++)
		for (int j=0;j<edges;j++){
			Array<int> v;
			v.add(_cyl_vert(i+1, j+1));
			v.add(_cyl_vert(i, j+1));
			v.add(_cyl_vert(i, j));
			v.add(_cyl_vert(i+1, j));
			Array<vector> _sv;
			_sv.add(_cyl_svert(i+1, j+1));
			_sv.add(_cyl_svert(i, j+1));
			_sv.add(_cyl_svert(i, j));
			_sv.add(_cyl_svert(i+1, j));
			AddPolygonSingleTexture(v, _sv);
		}

	if (closed)
		return;

// the endings
	int nv2 = Vertex.num;

	// center points
	AddVertex(inter.get(0));
	AddVertex(inter.get(1));

	// skin vertices
	sv.clear();
	for (int i=0;i<2;i++){
		sv.add(vector(0.5f,0.5f,0));
		for (int j=0;j<edges;j++){
			float w=pi*2*(float)j/(float)edges;
			sv.add(vector(0.5f+(float)sin(w)/2,0.5f+(float)cos(w)/2,0));
		}
	}

	// triangles
	for (int j=0;j<edges;j++){
		Array<int> v;
		v.add(nv2);
		v.add(j);
		v.add((j+1)%edges);
		Array<vector> _sv;
		_sv.add(sv[0]);
		_sv.add(sv[1+j]);
		_sv.add(sv[1+(j+1)%edges]);
		AddPolygonSingleTexture(v, _sv);
	}
	for (int j=0;j<edges;j++){
		Array<int> v;
		v.add(nv2+1);
		v.add(nv2-edges+(j+1)%edges);
		v.add(nv2-edges+j);
		Array<vector> _sv;
		_sv.add(sv[edges+1]);
		_sv.add(sv[edges+2+(j+1)%edges]);
		_sv.add(sv[edges+2+j]);
		AddPolygonSingleTexture(v, _sv);
	}
}

