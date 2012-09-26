/*
 * ActionModelAddBall.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionModelAddBall.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Triangle/ActionModelAddTriangleSingleTexture.h"
#include "../Surface/ActionModelSurfaceAutoWeld.h"
#include "../../../../Data/Model/DataModel.h"


#define ball_ang(x, y)	vector((float)(y)/(float)num_y,(float)(x)/(float)num_x,0)

ActionModelAddBall::ActionModelAddBall(DataModel *m, const vector &_pos, float _radius, int _num_x, int _num_y, bool _as_sphere)
{
	pos = _pos;
	radius = _radius;
	num_x = _num_x;
	num_y = _num_y;
	as_sphere = _as_sphere;
}

void *ActionModelAddBall::compose(Data *d)
{
#if 0
	DataModel *m = dynamic_cast<DataModel*>(d);
	int nv = m->Vertex.num;
	int material = m->CurrentMaterial;
	// sphere = "blown up cube"
	if (as_sphere){
		for (int f=0;f<6;f++){
	//		nv = Vertex.num;
			matrix mat;
			if (f==0)	MatrixIdentity(mat);
			if (f==1)	MatrixRotationX(mat, pi/2);
			if (f==2)	MatrixRotationX(mat,-pi/2);
			if (f==3)	MatrixRotationX(mat, pi);
			if (f==4)	MatrixRotationY(mat, pi/2);
			if (f==5)	MatrixRotationY(mat,-pi/2);
			// create new vertices
			for (int x=0;x<num_x+1;x++)
				for (int y=0;y<num_x+1;y++){
					vector dp=vector(float(x*2-num_x),float(y*2-num_x),float(num_x));
					dp.normalize();
					dp = mat * dp;
					AddSubAction(new ActionModelAddVertex(pos + radius * dp), m);
				}
			// create new triangles
			for (int x=0;x<num_x;x++)
				for (int y=0;y<num_x;y++){
					int a=(num_x+1)* x   +y;
					int b=(num_x+1)* x   +y+1;
					int c=(num_x+1)*(x+1)+y;
					int d=(num_x+1)*(x+1)+y+1;
					vector sva = vector((float) x   /(float)num_x,(float) y   /(float)num_x,0);
					vector svb = vector((float) x   /(float)num_x,(float)(y+1)/(float)num_x,0);
					vector svc = vector((float)(x+1)/(float)num_x,(float) y   /(float)num_x,0);
					vector svd = vector((float)(x+1)/(float)num_x,(float)(y+1)/(float)num_x,0);
					AddSubAction(new ActionModelAddTriangleSingleTexture(m, nv+a,nv+c,nv+d, material, sva,svc,svd), m);
					AddSubAction(new ActionModelAddTriangleSingleTexture(m, nv+a,nv+d,nv+b, material, sva,svd,svb), m);
				}
			nv += (num_x+1) * (num_x + 1);
		}

		// weld together
		for (int f=1;f<6;f++)
			AddSubAction(new ActionModelSurfaceAutoWeld(m, m->Surface.num - 2, m->Surface.num - 1, radius / (float)num_x * 0.01f, true), m);

	// ball from disks
	}else{
		// create new vertices
		AddSubAction(new ActionModelAddVertex(pos + e_y * radius), m);
		AddSubAction(new ActionModelAddVertex(pos - e_y * radius), m);
		for (int x=0;x<num_x+1;x++)
			for (int y=0;y<num_y+1;y++){
				if ((x>0)&&(x<num_x)&&(y<num_y))
					AddSubAction(new ActionModelAddVertex(pos + radius * vector(pi*(float(x)-(float)num_x/2.0f)/num_x,pi*2.0f* y/num_y,0).ang2dir()), m);
			}
		// create new triangles
		for (int y=0;y<num_y;y++)
			AddSubAction(new ActionModelAddTriangleSingleTexture(
					m,
					nv+0				,nv+2+y				,nv+2+(y+1)%num_y,
					material,
					ball_ang(0, y+1),	ball_ang(1, y),	ball_ang(1, y+1)), m);
		for (int y=0;y<num_y;y++)
			AddSubAction(new ActionModelAddTriangleSingleTexture(
					m,
					nv+2+num_y*(num_x-2)+y	,nv+1				,nv+2+num_y*(num_x-2)+(y+1)%num_y,
					material,
					ball_ang(num_x-1, y),		ball_ang(num_x, y),	ball_ang(num_x-1, y+1)), m);
		for (int x=1;x<num_x-1;x++)
			for (int y=0;y<num_y;y++){
				vector sva = ball_ang(x  , y  );
				vector svb = ball_ang(x  , y+1);
				vector svc = ball_ang(x+1, y  );
				vector svd = ball_ang(x+1, y+1);
				AddSubAction(new ActionModelAddTriangleSingleTexture(
						m,
						nv+2 +  num_y *(x-1)+ y	,nv+2 + num_y * x    +  y	,nv+2 +  num_y   *(x-1) + (y+1)%num_y,
						material,
						sva, svc, svb), m);
				AddSubAction(new ActionModelAddTriangleSingleTexture(
						m,
						nv+2 +  num_y * x   + y		,nv+2 + num_y * x    + (y+1)%num_y	,nv+2 +  num_y   *(x-1) + (y+1)%num_y,
						material,
						svc, svd, svb), m);
			}
	}
	return &m->Surface.back();
#endif
}
