/*
 * ModelGeometrySphere.cpp
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#include "ModelGeometrySphere.h"
#include "../DataModel.h"

ModelGeometrySphere::ModelGeometrySphere(const vector &pos, float radius, int num)
{
	int nv = 0;

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
				AddVertex(pos + radius * dp);
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
				AddPolygonSingleTexture(v, sv);
			}
		nv += (num + 1) * (num + 1);
	}
	Weld(radius / num * 0.01f);
}

