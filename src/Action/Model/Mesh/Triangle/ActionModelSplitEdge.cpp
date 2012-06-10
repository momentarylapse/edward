/*
 * ActionModelSplitEdge.cpp
 *
 *  Created on: 09.06.2012
 *      Author: michi
 */

#include "ActionModelSplitEdge.h"
#include "../Surface/Helper/ActionModelSurfaceDeleteTriangle.h"
#include "../Surface/Helper/ActionModel__SurfaceAddTriangle.h"
#include "../Vertex/ActionModelAddVertex.h"
#include <assert.h>

inline int tria_sort_vert_by_edge(const ModeModelTriangle &t, const ModeModelEdge &e, int v[3])
{
	for (int k=0;k<3;k++)
		if ((t.Vertex[k] != e.Vertex[0]) && (t.Vertex[k] != e.Vertex[1])){
			v[0] = t.Vertex[k];
			v[1] = t.Vertex[(k + 1) % 3];
			v[2] = t.Vertex[(k + 2) % 3];
			return k;
		}
	assert(0 && "funny triangle");
	return -1;
}

ActionModelSplitEdge::ActionModelSplitEdge(DataModel *m, int _surface, int _edge, const vector &_pos)
{
	assert(_surface >= 0);
	assert(_surface < m->Surface.num);
	ModeModelSurface &s = m->Surface[_surface];
	assert(_edge >= 0);
	assert(_edge < s.Edge.num);

	ModeModelEdge &e = s.Edge[_edge];
	assert(e.Triangle[0] >= 0);

	// copy old triangles data
	int nt0 = e.Triangle[0];
	int nt1 = e.Triangle[1];
	int v0[3], v1[3];

	int material0 = s.Triangle[nt0].Material;
	int material1;
	vector sv[8][MODEL_MAX_TEXTURES];
	int d0 = tria_sort_vert_by_edge(s.Triangle[nt0], e, v0);
	for (int k=0;k<3;k++)
		for (int l=0;l<MODEL_MAX_TEXTURES;l++)
			sv[k][l] = s.Triangle[nt0].SkinVertex[l][(k + d0) % 3];

	if (nt1 >= 0){
		material1 = s.Triangle[nt1].Material;
		int d1 = tria_sort_vert_by_edge(s.Triangle[nt1], e, v1);
		for (int k=0;k<3;k++)
			for (int l=0;l<MODEL_MAX_TEXTURES;l++)
				sv[3 + k][l] = s.Triangle[nt1].SkinVertex[l][(k + d1) % 3];
	}

	// bary centric
	float f = VecFactorBetween(_pos, m->Vertex[e.Vertex[0]].pos, m->Vertex[e.Vertex[1]].pos);
	if (e.Vertex[0] != v0[1]) // TODO ...
		f = 1 - f;
	for (int l=0;l<MODEL_MAX_TEXTURES;l++)
		sv[6][l] = sv[1][l] * (1 - f) + sv[2][l] * f;

	// delete old triangle(s)
	if (nt1 >= 0){
		AddSubAction(new ActionModelSurfaceDeleteTriangle(_surface, nt1), m);
		if (nt0 > nt1)
			nt0 --;
	}
	AddSubAction(new ActionModelSurfaceDeleteTriangle(_surface, nt0), m);

	// create new vertex
	AddSubAction(new ActionModelAddVertex(_pos), m);
	int v = m->Vertex.num - 1;

	// create 2-4 new triangles
	AddSubAction(new ActionModel__SurfaceAddTriangle(_surface, v0[0], v0[1], v, material0, sv[0], sv[1], sv[6]), m);
	AddSubAction(new ActionModel__SurfaceAddTriangle(_surface, v0[2], v0[0], v, material0, sv[2], sv[0], sv[6]), m);
	if (nt1 >= 0){
		AddSubAction(new ActionModel__SurfaceAddTriangle(_surface, v1[0], v1[1], v, material1, sv[3], sv[4], sv[6]), m);
		AddSubAction(new ActionModel__SurfaceAddTriangle(_surface, v1[2], v1[0], v, material1, sv[5], sv[3], sv[6]), m);
	}
}

ActionModelSplitEdge::~ActionModelSplitEdge()
{
}
