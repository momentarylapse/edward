/*
 * ActionModelAddBall.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionModelAddBall.h"
#include "ActionModelAddVertex.h"
#include "ActionModelAddTriangle.h"


#define ball_ang(x, y)	vector((float)(y)/(float)_num_y,(float)(x)/(float)_num_x,0)

ActionModelAddBall::ActionModelAddBall(const vector &_pos, float _radius, int _num_x, int _num_y, bool _as_sphere, int nv)
{
	// sphere = "blown up cube"
	if (_as_sphere){
		for (int f=0;f<6;f++){
	//		nv = Vertex.num;
			matrix m;
			if (f==0)	MatrixIdentity(m);
			if (f==1)	MatrixRotationX(m, pi/2);
			if (f==2)	MatrixRotationX(m,-pi/2);
			if (f==3)	MatrixRotationX(m, pi);
			if (f==4)	MatrixRotationY(m, pi/2);
			if (f==5)	MatrixRotationY(m,-pi/2);
			// create new vertices
			for (int x=0;x<_num_x+1;x++)
				for (int y=0;y<_num_x+1;y++){
					vector dp=vector(float(x*2-_num_x),float(y*2-_num_x),float(_num_x));
					VecNormalize(dp);
					VecTransform(dp,m,dp);
					action.add(new ActionModelAddVertex(_pos + _radius * dp));
				}
			// create new triangles
			for (int x=0;x<_num_x;x++)
				for (int y=0;y<_num_x;y++){
					int a=(_num_x+1)* x   +y;
					int b=(_num_x+1)* x   +y+1;
					int c=(_num_x+1)*(x+1)+y;
					int d=(_num_x+1)*(x+1)+y+1;
					vector sva = vector((float) x   /(float)_num_x,(float) y   /(float)_num_x,0);
					vector svb = vector((float) x   /(float)_num_x,(float)(y+1)/(float)_num_x,0);
					vector svc = vector((float)(x+1)/(float)_num_x,(float) y   /(float)_num_x,0);
					vector svd = vector((float)(x+1)/(float)_num_x,(float)(y+1)/(float)_num_x,0);
					action.add(new ActionModelAddTriangle(nv+a,nv+c,nv+d,sva,svc,svd));
					action.add(new ActionModelAddTriangle(nv+a,nv+d,nv+b,sva,svd,svb));
				}
			nv += (_num_x+1) * (_num_x + 1);
		}
		/*for (int f=1;f<6;f++)
			SurfaceAutoWeld(&Surface[Surface.num - 2], &Surface.back(), radius / (float)nx * 0.01f);
		s = &Surface.back();*/

	// ball from disks
	}else{
		// create new vertices
		action.add(new ActionModelAddVertex(_pos + e_y * _radius));
		action.add(new ActionModelAddVertex(_pos - e_y * _radius));
		for (int x=0;x<_num_x+1;x++)
			for (int y=0;y<_num_y+1;y++){
				if ((x>0)&&(x<_num_x)&&(y<_num_y))
					action.add(new ActionModelAddVertex(_pos + _radius * VecAng2Dir(vector(pi*(float(x)-(float)_num_x/2.0f)/_num_x,pi*2.0f* y/_num_y,0))));
			}
		// create new triangles
		for (int y=0;y<_num_y;y++)
			action.add(new ActionModelAddTriangle(
					nv+0				,nv+2+y				,nv+2+(y+1)%_num_y,
					ball_ang(0, y+1),	ball_ang(1, y),	ball_ang(1, y+1)));
		for (int y=0;y<_num_y;y++)
			action.add(new ActionModelAddTriangle(
					nv+2+_num_y*(_num_x-2)+y	,nv+1				,nv+2+_num_y*(_num_x-2)+(y+1)%_num_y,
					ball_ang(_num_x-1, y),		ball_ang(_num_x, y),	ball_ang(_num_x-1, y+1)));
		for (int x=1;x<_num_x-1;x++)
			for (int y=0;y<_num_y;y++){
				vector sva = ball_ang(x  , y  );
				vector svb = ball_ang(x  , y+1);
				vector svc = ball_ang(x+1, y  );
				vector svd = ball_ang(x+1, y+1);
				action.add(new ActionModelAddTriangle(
						nv+2 +  _num_y *(x-1)+ y	,nv+2 + _num_y * x    +  y	,nv+2 +  _num_y   *(x-1) + (y+1)%_num_y,
						sva, svc, svb));
				action.add(new ActionModelAddTriangle(
						nv+2 +  _num_y * x   + y		,nv+2 + _num_y * x    + (y+1)%_num_y	,nv+2 +  _num_y   *(x-1) + (y+1)%_num_y,
						svc, svd, svb));
			}
	}
}

ActionModelAddBall::~ActionModelAddBall()
{
}
