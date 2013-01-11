/*
 * ModelGeometryPlatonic.cpp
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#include "ModelGeometryPlatonic.h"
#include "../DataModel.h"

ModelGeometryPlatonic::ModelGeometryPlatonic(const vector &pos, float radius, int type)
{
	if (type == 4)
		AddTetrahedron(pos, radius);
	else if (type == 8)
		AddOctahedron(pos, radius);
	else if (type == 12)
		AddDodecahedron(pos, radius);
	else if (type == 20)
		AddIcosahedron(pos, radius);
}


void ModelGeometryPlatonic::AddTetrahedron(const vector &pos, float radius)
{
	int nv = Vertex.num;

	float r = radius / sqrt(3/2);
	AddVertex(pos + vector( 1, 0, -1/sqrt(2)) * r);
	AddVertex(pos + vector(-1, 0, -1/sqrt(2)) * r);
	AddVertex(pos + vector( 0, 1,  1/sqrt(2)) * r);
	AddVertex(pos + vector( 0,-1,  1/sqrt(2)) * r);
	Add3(nv, 0, 1, 2);
	Add3(nv, 1, 0, 3);
	Add3(nv, 0, 2, 3);
	Add3(nv, 1, 3, 2);
}

void ModelGeometryPlatonic::AddOctahedron(const vector &pos, float radius)
{
	int nv = Vertex.num;

	float r = radius;
	AddVertex(pos + vector( 1, 0, 0) * r);
	AddVertex(pos + vector(-1, 0, 0) * r);
	AddVertex(pos + vector( 0, 1, 0) * r);
	AddVertex(pos + vector( 0,-1, 0) * r);
	AddVertex(pos + vector( 0, 0, 1) * r);
	AddVertex(pos + vector( 0, 0,-1) * r);
	Add3(nv, 0, 2, 4);
	Add3(nv, 4, 2, 1);
	Add3(nv, 1, 2, 5);
	Add3(nv, 5, 2, 0);
	Add3(nv, 4, 3, 0);
	Add3(nv, 0, 3, 5);
	Add3(nv, 5, 3, 1);
	Add3(nv, 1, 3, 4);
}

void ModelGeometryPlatonic::AddDodecahedron(const vector &pos, float radius)
{
	int nv = Vertex.num;

	float phi = (1 + sqrt(5))/2;
	float r = radius / sqrt(3);
	AddVertex(pos + vector( 1, 1, 1) * r);
	AddVertex(pos + vector( 1, 1,-1) * r);
	AddVertex(pos + vector( 1,-1, 1) * r);
	AddVertex(pos + vector( 1,-1,-1) * r);
	AddVertex(pos + vector(-1, 1, 1) * r);
	AddVertex(pos + vector(-1, 1,-1) * r);
	AddVertex(pos + vector(-1,-1, 1) * r);
	AddVertex(pos + vector(-1,-1,-1) * r);
	AddVertex(pos + vector(0, 1/phi, phi) * r);
	AddVertex(pos + vector(0, 1/phi,-phi) * r);
	AddVertex(pos + vector(0,-1/phi, phi) * r);
	AddVertex(pos + vector(0,-1/phi,-phi) * r);
	AddVertex(pos + vector( 1/phi, phi,0) * r);
	AddVertex(pos + vector( 1/phi,-phi,0) * r);
	AddVertex(pos + vector(-1/phi, phi,0) * r);
	AddVertex(pos + vector(-1/phi,-phi,0) * r);
	AddVertex(pos + vector( phi,0, 1/phi) * r);
	AddVertex(pos + vector(-phi,0, 1/phi) * r);
	AddVertex(pos + vector( phi,0,-1/phi) * r);
	AddVertex(pos + vector(-phi,0,-1/phi) * r);
	Add5(nv, 14, 12, 1, 9, 5);
	Add5(nv, 12, 14, 4, 8, 0);
	Add5(nv, 13, 15, 7, 11, 3);
	Add5(nv, 15, 13, 2, 10, 6);
	Add5(nv, 16, 18, 1, 12, 0);
	Add5(nv, 18, 16, 2, 13, 3);
	Add5(nv, 19, 17, 4, 14, 5);
	Add5(nv, 17, 19, 7, 15, 6);
	Add5(nv, 9, 11, 7, 19, 5);
	Add5(nv, 11, 9, 1, 18, 3);
	Add5(nv, 10, 8, 4, 17, 6);
	Add5(nv, 8, 10, 2, 16, 0);
}

void ModelGeometryPlatonic::AddIcosahedron(const vector &pos, float radius)
{
	int nv = Vertex.num;

	float phi = (1 + sqrt(5))/2;
	float r = radius / sqrt(phi * sqrt(5));
	AddVertex(pos + vector( 0, 1, phi) * r);
	AddVertex(pos + vector( 0, 1,-phi) * r);
	AddVertex(pos + vector( 0,-1, phi) * r);
	AddVertex(pos + vector( 0,-1,-phi) * r);
	AddVertex(pos + vector( 1, phi, 0) * r);
	AddVertex(pos + vector( 1,-phi, 0) * r);
	AddVertex(pos + vector(-1, phi, 0) * r);
	AddVertex(pos + vector(-1,-phi, 0) * r);
	AddVertex(pos + vector( phi, 0, 1) * r);
	AddVertex(pos + vector(-phi, 0, 1) * r);
	AddVertex(pos + vector( phi, 0,-1) * r);
	AddVertex(pos + vector(-phi, 0,-1) * r);
	Add3(nv, 0, 2, 8);
	Add3(nv, 2, 0, 9);
	Add3(nv, 3, 1,10);
	Add3(nv, 1, 3,11);
	Add3(nv, 4, 6, 0);
	Add3(nv, 6, 4, 1);
	Add3(nv, 7, 5, 2);
	Add3(nv, 5, 7, 3);
	Add3(nv, 8,10, 4);
	Add3(nv,10, 8, 5);
	Add3(nv,11, 9, 6);
	Add3(nv, 9,11, 7);
	Add3(nv, 4, 0, 8);
	Add3(nv, 0, 6, 9);
	Add3(nv, 2, 5, 8);
	Add3(nv, 7, 2, 9);
	Add3(nv, 3, 7,11);
	Add3(nv, 5, 3,10);
	Add3(nv, 6, 1,11);
	Add3(nv, 1, 4,10);
}
