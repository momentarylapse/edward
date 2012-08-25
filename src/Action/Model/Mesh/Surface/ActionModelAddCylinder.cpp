/*
 * ActionModelAddCylinder.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionModelAddCylinder.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Triangle/ActionModelAddTriangleSingleTexture.h"
#include "../../../../Data/Model/DataModel.h"

#define _cyl_vert(i, j)         ( edges      * (i) +(j) % edges) + nv
#define _cyl_svert(i, j)        sv[(edges + 1) * (i) +(j) % (edges + 1)]

ActionModelAddCylinder::ActionModelAddCylinder(DataModel *m, Array<vector> &pos, Array<float> &_radius, int rings, int edges, bool closed)
{
	int nv = m->Vertex.num;
	int material = m->CurrentMaterial;

	Interpolator<float> inter_r(Interpolator<float>::TYPE_CUBIC_SPLINE_NOTANG);
	foreach(_radius, r)
		inter_r.add(r);

	// vertices (interpolated on path)
	Interpolator<vector> inter(Interpolator<vector>::TYPE_CUBIC_SPLINE_NOTANG);
	foreach(pos, p)
		inter.add(p);
	Array<vector> sv;
	vector r_last = v0;
	for (int i=0;i<=rings;i++){
		// interpolated point on path
		float t = (float)i / (float)rings;
		vector p0 = inter.get(t);
		vector dir = inter.get_tang(t);

		// moving frame
		vector u = r_last ^ dir;
		if (i == 0)
			u = VecOrtho(dir);
		VecNormalize(u);
		vector r = dir ^ u;
		VecNormalize(r);
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
			AddSubAction(new ActionModelAddTriangleSingleTexture(m,
					_cyl_vert(i, j),  _cyl_vert(i+1, j+1),  _cyl_vert(i, j+1), material,
					_cyl_svert(i, j), _cyl_svert(i+1, j+1), _cyl_svert(i, j+1)), m);
			AddSubAction(new ActionModelAddTriangleSingleTexture(m,
					_cyl_vert(i, j),  _cyl_vert(i+1, j),  _cyl_vert(i+1, j+1), material,
					_cyl_svert(i, j), _cyl_svert(i+1, j), _cyl_svert(i+1, j+1)), m);
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
				AddSubAction(new ActionModelAddTriangleSingleTexture(m,
						nv2	,nv+j	,nv+(j+1)%edges, material,
						sv[0], sv[1+j], sv[1+(j+1)%edges]), m);
				AddSubAction(new ActionModelAddTriangleSingleTexture(m,
						nv2+1	,nv2-edges+(j+1)%edges	,nv2-edges+j, material,
						sv[edges+1] ,sv[edges+2+(j+1)%edges], sv[edges+2+j]), m);
			}
		}
}

ActionModelAddCylinder::~ActionModelAddCylinder()
{
}

void *ActionModelAddCylinder::execute_return(Data *d)
{	return &(dynamic_cast<DataModel*>(d))->Surface.back();	}
