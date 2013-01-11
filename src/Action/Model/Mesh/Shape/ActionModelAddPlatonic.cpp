/*
 * ActionModelAddPlatonic.cpp
 *
 *  Created on: 24.12.2012
 *      Author: michi
 */

#include "ActionModelAddPlatonic.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Polygon/ActionModelAddPolygonAutoSkin.h"
#include "../../../../Data/Model/DataModel.h"
#include "ActionModelAddTeapot.h"

ActionModelAddPlatonic::ActionModelAddPlatonic(const vector &_pos, float _radius, int _type)
{
	pos = _pos;
	radius = _radius;
	type = _type;
}


void ActionModelAddPlatonic::add3(int nv, int v0, int v1, int v2, DataModel *m)
{
	Array<int> v;
	v.add(nv + v0);
	v.add(nv + v1);
	v.add(nv + v2);
	AddSubAction(new ActionModelAddPolygonAutoSkin(v), m);
}

void ActionModelAddPlatonic::add5(int nv, int v0, int v1, int v2, int v3, int v4, DataModel *m)
{
	Array<int> v;
	v.add(nv + v0);
	v.add(nv + v1);
	v.add(nv + v2);
	v.add(nv + v3);
	v.add(nv + v4);
	AddSubAction(new ActionModelAddPolygonAutoSkin(v), m);
}

void *ActionModelAddPlatonic::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	if (type == 4)
		AddTetrahedron(m);
	else if (type == 8)
		AddOctahedron(m);
	else if (type == 12)
		AddDodecahedron(m);
	else if (type == 20)
		AddIcosahedron(m);
	else if (type == 306)
		AddSubAction(new ActionModelAddTeapot(pos, radius, 4), m);
	else
		throw ActionException("unknown platonic: " + i2s(type));

	return &m->Surface.back();
}

void ActionModelAddPlatonic::AddTetrahedron(DataModel *m)
{
	int nv = m->Vertex.num;

	float r = radius / sqrt(3/2);
	AddSubAction(new ActionModelAddVertex(pos + vector( 1, 0, -1/sqrt(2)) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(-1, 0, -1/sqrt(2)) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( 0, 1,  1/sqrt(2)) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( 0,-1,  1/sqrt(2)) * r), m);
	add3(nv, 0, 1, 2, m);
	add3(nv, 1, 0, 3, m);
	add3(nv, 0, 2, 3, m);
	add3(nv, 1, 3, 2, m);
}

void ActionModelAddPlatonic::AddOctahedron(DataModel *m)
{
	int nv = m->Vertex.num;

	float r = radius;
	AddSubAction(new ActionModelAddVertex(pos + vector( 1, 0, 0) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(-1, 0, 0) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( 0, 1, 0) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( 0,-1, 0) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( 0, 0, 1) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( 0, 0,-1) * r), m);
	add3(nv, 0, 2, 4, m);
	add3(nv, 4, 2, 1, m);
	add3(nv, 1, 2, 5, m);
	add3(nv, 5, 2, 0, m);
	add3(nv, 4, 3, 0, m);
	add3(nv, 0, 3, 5, m);
	add3(nv, 5, 3, 1, m);
	add3(nv, 1, 3, 4, m);
}

void ActionModelAddPlatonic::AddDodecahedron(DataModel *m)
{
	int nv = m->Vertex.num;

	float phi = (1 + sqrt(5))/2;
	float r = radius / sqrt(3);
	AddSubAction(new ActionModelAddVertex(pos + vector( 1, 1, 1) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( 1, 1,-1) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( 1,-1, 1) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( 1,-1,-1) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(-1, 1, 1) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(-1, 1,-1) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(-1,-1, 1) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(-1,-1,-1) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(0, 1/phi, phi) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(0, 1/phi,-phi) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(0,-1/phi, phi) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(0,-1/phi,-phi) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( 1/phi, phi,0) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( 1/phi,-phi,0) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(-1/phi, phi,0) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(-1/phi,-phi,0) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( phi,0, 1/phi) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(-phi,0, 1/phi) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( phi,0,-1/phi) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(-phi,0,-1/phi) * r), m);
	add5(nv, 14, 12, 1, 9, 5, m);
	add5(nv, 12, 14, 4, 8, 0, m);
	add5(nv, 13, 15, 7, 11, 3, m);
	add5(nv, 15, 13, 2, 10, 6, m);
	add5(nv, 16, 18, 1, 12, 0, m);
	add5(nv, 18, 16, 2, 13, 3, m);
	add5(nv, 19, 17, 4, 14, 5, m);
	add5(nv, 17, 19, 7, 15, 6, m);
	add5(nv, 9, 11, 7, 19, 5, m);
	add5(nv, 11, 9, 1, 18, 3, m);
	add5(nv, 10, 8, 4, 17, 6, m);
	add5(nv, 8, 10, 2, 16, 0, m);
}

void ActionModelAddPlatonic::AddIcosahedron(DataModel *m)
{
	int nv = m->Vertex.num;

	float phi = (1 + sqrt(5))/2;
	float r = radius / sqrt(phi * sqrt(5));
	AddSubAction(new ActionModelAddVertex(pos + vector( 0, 1, phi) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( 0, 1,-phi) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( 0,-1, phi) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( 0,-1,-phi) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( 1, phi, 0) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( 1,-phi, 0) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(-1, phi, 0) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(-1,-phi, 0) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( phi, 0, 1) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(-phi, 0, 1) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector( phi, 0,-1) * r), m);
	AddSubAction(new ActionModelAddVertex(pos + vector(-phi, 0,-1) * r), m);
	add3(nv, 0, 2, 8, m);
	add3(nv, 2, 0, 9, m);
	add3(nv, 3, 1,10, m);
	add3(nv, 1, 3,11, m);
	add3(nv, 4, 6, 0, m);
	add3(nv, 6, 4, 1, m);
	add3(nv, 7, 5, 2, m);
	add3(nv, 5, 7, 3, m);
	add3(nv, 8,10, 4, m);
	add3(nv,10, 8, 5, m);
	add3(nv,11, 9, 6, m);
	add3(nv, 9,11, 7, m);
	add3(nv, 4, 0, 8, m);
	add3(nv, 0, 6, 9, m);
	add3(nv, 2, 5, 8, m);
	add3(nv, 7, 2, 9, m);
	add3(nv, 3, 7,11, m);
	add3(nv, 5, 3,10, m);
	add3(nv, 6, 1,11, m);
	add3(nv, 1, 4,10, m);
}
