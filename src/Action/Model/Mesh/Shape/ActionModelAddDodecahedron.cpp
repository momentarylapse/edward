/*
 * ActionModelAddDodecahedron.cpp
 *
 *  Created on: 24.12.2012
 *      Author: michi
 */

#include "ActionModelAddDodecahedron.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Polygon/ActionModelAddPolygonAutoSkin.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelAddDodecahedron::ActionModelAddDodecahedron(const vector &_pos, float _radius)
{
	pos = _pos;
	radius = _radius;
}


void ActionModelAddDodecahedron::add5(int nv, int v0, int v1, int v2, int v3, int v4, DataModel *m)
{
	Array<int> v;
	v.add(nv + v0);
	v.add(nv + v1);
	v.add(nv + v2);
	v.add(nv + v3);
	v.add(nv + v4);
	AddSubAction(new ActionModelAddPolygonAutoSkin(v), m);
}

void *ActionModelAddDodecahedron::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

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

	/*int nv = m->Vertex.num;
	int material = m->CurrentMaterial;
	// sphere = "blown up cube"
	for (int f=0;f<6;f++){
	//	nv = Vertex.num;
		matrix mat;
		if (f==0)	MatrixIdentity(mat);
		if (f==1)	MatrixRotationX(mat, pi/2);
		if (f==2)	MatrixRotationX(mat,-pi/2);
		if (f==3)	MatrixRotationY(mat, pi/2);
		if (f==4)	MatrixRotationX(mat, pi);
		if (f==5)	MatrixRotationY(mat,-pi/2);
		// create new vertices
		for (int x=0;x<num+1;x++)
			for (int y=0;y<num+1;y++){
				vector dp=vector(float(x*2-num),float(y*2-num),float(num));
				dp.normalize();
				dp = mat * dp;
				AddSubAction(new ActionModelAddVertex(pos + radius * dp), m);
			}
		// create new triangles
		for (int x=0;x<num;x++)
			for (int y=0;y<num;y++){
				Array<int> v;
				v.add(nv + (num+1)* x   +y+1);
				v.add(nv + (num+1)* x   +y);
				v.add(nv + (num+1)*(x+1)+y);
				v.add(nv + (num+1)*(x+1)+y+1);
				Array<vector> sv;
				sv.add(vector((float) x   /(float)num,(float)(y+1)/(float)num,0));
				sv.add(vector((float) x   /(float)num,(float) y   /(float)num,0));
				sv.add(vector((float)(x+1)/(float)num,(float) y   /(float)num,0));
				sv.add(vector((float)(x+1)/(float)num,(float)(y+1)/(float)num,0));
				AddSubAction(new ActionModelAddPolygonSingleTexture(v, material, sv), m);
			}
		nv += (num + 1) * (num + 1);
	}

	// weld together
	for (int f=1;f<6;f++)
		AddSubAction(new ActionModelSurfaceAutoWeld(m->Surface.num - 2, m->Surface.num - 1, radius / (float)num * 0.01f), m);
*/
	return &m->Surface.back();
}
