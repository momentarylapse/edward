/*
 * ActionModelAddCylinder.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionModelAddCylinder.h"
#include "ActionModelAddVertex.h"
#include "ActionModelAddTriangleSingleTexture.h"

ActionModelAddCylinder::ActionModelAddCylinder(DataModel *m, Array<vector> &pos, float radius, int rings, int edges, bool closed)
{
	int nv = m->Vertex.num;
	int material = m->CurrentMaterial;

	// vertices (interpolated on path)
	Interpolator inter(Interpolator::TYPE_CUBIC_SPLINE_NOTANG);
	foreach(pos, p)
		inter.Add(p);
	int n = (pos.num - 1) * rings;
	Array<vector> sv;
	vector r_last = v0;
	for (int i=0;i<=n;i++){
		// interpolated point on path
		float t = (float)i / (float)n;
		vector p0 = inter.Get(t);
		vector dir = inter.GetTang(t);

		// moving frame
		vector u = r_last ^ dir;
		if (i == 0)
			u = VecOrtho(dir);
		VecNormalize(u);
		vector r = dir ^ u;
		VecNormalize(r);
		r_last = r;

		// vertex ring
		for (int j=0;j<edges+1;j++){
			float w = pi*2*(float)j/(float)edges;
			vector p = p0+((float)sin(w)*u+(float)cos(w)*r)*radius;
			if (j < edges)
				AddSubAction(new ActionModelAddVertex(p), m);
			sv.add(vector((float)j/(float)edges,t,0));
		}
	}

// the curved surface
	for (int i=0;i<n;i++)
		for (int j=0;j<edges;j++){
			int _a = edges * i   +j;
			int _b = edges *(i+1)+j;
			int _c = edges * i   +(j+1) % edges;
			int _d = edges *(i+1)+(j+1) % edges;
			int _cs = edges * i   +j+1;
			int _ds = edges *(i+1)+j+1;
			AddSubAction(new ActionModelAddTriangleSingleTexture(
					m,
					nv+_a,nv+_d,nv+_c,
					material,
					sv[_a]   ,sv[_ds]   ,sv[_cs]   ), m);
			AddSubAction(new ActionModelAddTriangleSingleTexture(
					m,
					nv+_a,nv+_b,nv+_d,
					material,
					sv[_a]   ,sv[_b]   ,sv[_ds]   ), m);
		}

// the endings
	if (closed){
		int nv2 = m->Vertex.num;

		// center points
		AddSubAction(new ActionModelAddVertex(inter.Get(0)), m);
		AddSubAction(new ActionModelAddVertex(inter.Get(1)), m);

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
				AddSubAction(new ActionModelAddTriangleSingleTexture(
						m,
						nv2	,nv+j	,nv+(j+1)%edges,
						material,
						sv[0], sv[1+j], sv[1+(j+1)%edges]), m);
				AddSubAction(new ActionModelAddTriangleSingleTexture(
						m,
						nv2+1	,nv2-edges+(j+1)%edges	,nv2-edges+j,
						material,
						sv[edges+1] ,sv[edges+2+(j+1)%edges], sv[edges+2+j]), m);
			}
		}
}

ActionModelAddCylinder::~ActionModelAddCylinder()
{
}
