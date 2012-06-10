/*
 * ActionModelSurfaceRelinkTriangle.cpp
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#include "ActionModelSurfaceRelinkTriangle.h"
#include "ActionModel__SurfaceDeleteTriangle.h"
#include "ActionModel__SurfaceAddTriangle.h"

ActionModelSurfaceRelinkTriangle::ActionModelSurfaceRelinkTriangle(DataModel *m, int _surface, int _triangle, int a, int b, int c)
{
	// old triangle data
	int material = m->Surface[_surface].Triangle[_triangle].Material;
	vector sv[3][MODEL_MAX_TEXTURES];
	for (int k=0;k<3;k++)
		for (int l=0;l<MODEL_MAX_TEXTURES;l++)
			sv[k][l] = m->Surface[_surface].Triangle[_triangle].SkinVertex[l][k];

	// delete old triangle
	AddSubAction(new ActionModel__SurfaceDeleteTriangle(_surface, _triangle), m);

	// create new triangle
	AddSubAction(new ActionModel__SurfaceAddTriangle(_surface, a, b, c, material, sv[0], sv[1], sv[2]), m);
}

ActionModelSurfaceRelinkTriangle::~ActionModelSurfaceRelinkTriangle()
{
}