/*
 * ActionModelAddCylinder.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionModelAddCylinder.h"
#include "ActionModelAddVertex.h"
#include "ActionModelAddTriangle.h"

ActionModelAddCylinder::ActionModelAddCylinder(DataModel *m, const vector &pos, const vector &length, float radius, int rings, int edges, bool closed)
{
	if (radius < 0)
		radius = -radius;

	vector u = VecOrtho(length);
	VecNormalize(u);
	vector r = length ^ u;
	VecNormalize(r);

	int nv = m->Vertex.num;

// the curved surface
	Array<vector> sv;
	for (int i=0;i<edges+1;i++){
		float w = pi*2*(float)i/(float)edges;
		vector p = pos+((float)sin(w)*u+(float)cos(w)*r)*radius;
		for (int j=0;j<rings+1;j++){
			if (i<edges)
				AddSubAction(new ActionModelAddVertex(p+length*(float)j/(float)rings), m);
			sv.add(vector((float)i/(float)edges,(float)j/(float)rings,0));
		}
	}
	int n=(rings+1)*edges;
	for (int i=0;i<edges;i++)
		for (int j=0;j<rings;j++){
			int _a=(rings+1)* i   +j;
			int _b=(rings+1)*(i+1)+j;
			int _c=(rings+1)* i   +j+1;
			int _d=(rings+1)*(i+1)+j+1;
			AddSubAction(new ActionModelAddTriangle(
					m,
					nv+(_a%n),nv+(_c%n),nv+(_d%n),
					sv[_a]   ,sv[_c]   ,sv[_d]   ), m);
			AddSubAction(new ActionModelAddTriangle(
					m,
					nv+(_a%n),nv+(_d%n),nv+(_b%n),
					sv[_a]   ,sv[_d]   ,sv[_b]   ), m);
		}
// the endings
	if (closed){
		sv.clear();
		AddSubAction(new ActionModelAddVertex(pos), m);
		AddSubAction(new ActionModelAddVertex(pos + length), m);
		for (int i=0;i<2;i++){
			sv.add(vector(0.5f,0.5f,0));
			for (int j=0;j<edges;j++){
				float w=pi*2*(float)j/(float)edges;
				sv.add(vector(0.5f+(float)sin(w)/2,0.5f+(float)cos(w)/2,0));
			}
		}
		for (int j=0;j<edges;j++){
			AddSubAction(new ActionModelAddTriangle(
					m,
					nv+(rings+1)*edges	,nv+j*(rings+1)						,nv+((j+1)%edges)*(rings+1),
					sv[0], sv[1+j], sv[1+(j+1)%edges]), m);
			AddSubAction(new ActionModelAddTriangle(
					m,
					nv+(rings+1)*edges+1,nv+((j+1)%edges)*(rings+1)+rings	,nv+j*(rings+1)+rings,
					sv[edges+1] ,sv[edges+2+(j+1)%edges], sv[edges+2+j]), m);
		}
	}
}

ActionModelAddCylinder::~ActionModelAddCylinder()
{
}
