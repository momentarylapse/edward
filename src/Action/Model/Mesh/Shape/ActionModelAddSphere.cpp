/*
 * ActionModelAddSphere.cpp
 *
 *  Created on: 24.12.2012
 *      Author: michi
 */

#include "ActionModelAddSphere.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Polygon/ActionModelAddPolygonSingleTexture.h"
#include "../Surface/ActionModelSurfaceAutoWeld.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelAddSphere::ActionModelAddSphere(const vector &_pos, float _radius, int _num)
{
	pos = _pos;
	radius = _radius;
	num = _num;
}

void *ActionModelAddSphere::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	int nv = m->Vertex.num;
	int material = m->CurrentMaterial;
	// sphere = "blown up cube"
	for (int f=0;f<6;f++){
	//	nv = Vertex.num;
		matrix mat;
		if (f==0)	MatrixIdentity(mat);
		if (f==1)	MatrixRotationX(mat, pi/2);
		if (f==2)	MatrixRotationX(mat,-pi/2);
		if (f==3)	MatrixRotationX(mat, pi);
		if (f==4)	MatrixRotationY(mat, pi/2);
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
		AddSubAction(new ActionModelSurfaceAutoWeld(m->Surface.num - 2, m->Surface.num - 1, radius / (float)num * 0.01f, true), m);

	return &m->Surface.back();
}
