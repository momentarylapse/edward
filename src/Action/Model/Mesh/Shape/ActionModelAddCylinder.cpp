/*
 * ActionModelAddCylinder.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionModelAddCylinder.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Polygon/ActionModelAddPolygonSingleTexture.h"
#include "../../../../Data/Model/DataModel.h"

#define _cyl_vert(i, j)         ( edges      * (i) +(j) % edges) + nv
#define _cyl_svert(i, j)        sv[(edges + 1) * (i) +(j) % (edges + 1)]

ActionModelAddCylinder::ActionModelAddCylinder(DataModel *m, Array<vector> &pos, Array<float> &_radius, int rings, int edges, bool closed)
{
	int nv = m->Vertex.num;
	int material = m->CurrentMaterial;

	Interpolator<float> inter_r(Interpolator<float>::TYPE_CUBIC_SPLINE_NOTANG);
	foreach(float r, _radius)
		inter_r.add(r);

	// vertices (interpolated on path)
	Interpolator<vector> inter(Interpolator<vector>::TYPE_CUBIC_SPLINE_NOTANG);
	foreach(vector &p, pos)
		inter.add(p);
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
				AddSubAction(new ActionModelAddVertex(p), m);
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
			AddSubAction(new ActionModelAddPolygonSingleTexture(v, material, _sv), m);
		}

// the endings
	if (closed){
		int nv2 = m->Vertex.num;

		// center points
		AddSubAction(new ActionModelAddVertex(inter.get(0)), m);
		AddSubAction(new ActionModelAddVertex(inter.get(1)), m);

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
			v.add(nv+j);
			v.add(nv+(j+1)%edges);
			Array<vector> _sv;
			_sv.add(sv[0]);
			_sv.add(sv[1+j]);
			_sv.add(sv[1+(j+1)%edges]);
			AddSubAction(new ActionModelAddPolygonSingleTexture(v, material, _sv), m);
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
			AddSubAction(new ActionModelAddPolygonSingleTexture(v, material, _sv), m);
		}
	}
}

ActionModelAddCylinder::~ActionModelAddCylinder()
{
}

void *ActionModelAddCylinder::compose(Data *d)
{	return &(dynamic_cast<DataModel*>(d))->Surface.back();	}
