/*
 * ActionModelSplitTriangle.cpp
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#include "ActionModelSplitTriangle.h"
#include "../Surface/Helper/ActionModel__SurfaceDeleteTriangle.h"
#include "../Surface/Helper/ActionModel__SurfaceAddTriangle.h"
#include "../Vertex/ActionModelAddVertex.h"

ActionModelSplitTriangle::ActionModelSplitTriangle(DataModel *m, int _surface, int _triangle, const vector &_pos)
{
	// old triangle data
	int va = m->Surface[_surface].Triangle[_triangle].Vertex[0];
	int vb = m->Surface[_surface].Triangle[_triangle].Vertex[1];
	int vc = m->Surface[_surface].Triangle[_triangle].Vertex[2];
	int material = m->Surface[_surface].Triangle[_triangle].Material;
	vector sv[4][MODEL_MAX_TEXTURES];
	for (int k=0;k<3;k++)
		for (int l=0;l<MODEL_MAX_TEXTURES;l++)
			sv[k][l] = m->Surface[_surface].Triangle[_triangle].SkinVertex[l][k];

	// bary centric
	float f, g;
	GetBaryCentric(_pos, m->Vertex[va].pos, m->Vertex[vb].pos, m->Vertex[vc].pos, f, g);
	for (int l=0;l<MODEL_MAX_TEXTURES;l++)
		sv[3][l] = sv[0][l] * (1 - f - g) + sv[1][l] * f + sv[2][l] * g;

	// delete old triangle
	AddSubAction(new ActionModel__SurfaceDeleteTriangle(_surface, _triangle), m);

	// create new vertex
	AddSubAction(new ActionModelAddVertex(_pos), m);
	int v = m->Vertex.num - 1;

	// create 3 new triangles
	AddSubAction(new ActionModel__SurfaceAddTriangle(_surface, va, vb, v, material, sv[0], sv[1], sv[3]), m);
	AddSubAction(new ActionModel__SurfaceAddTriangle(_surface, vb, vc, v, material, sv[1], sv[2], sv[3]), m);
	AddSubAction(new ActionModel__SurfaceAddTriangle(_surface, vc, va, v, material, sv[2], sv[0], sv[3]), m);
}

ActionModelSplitTriangle::~ActionModelSplitTriangle()
{
}
