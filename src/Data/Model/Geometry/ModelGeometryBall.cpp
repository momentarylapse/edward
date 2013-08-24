/*
 * ModelGeometryBall.cpp
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#include "ModelGeometryBall.h"
#include "../DataModel.h"

#define ball_ang(x, y)	vector((float)(y)/(float)num_y,(float)(x)/(float)num_x,0)

ModelGeometryBall::ModelGeometryBall(const vector &pos, float radius, int num_x, int num_y)
{
	// ball from disks

	// create new vertices
	AddVertex(pos + e_y * radius);
	AddVertex(pos - e_y * radius);
	for (int x=0;x<num_x+1;x++)
		for (int y=0;y<num_y+1;y++){
			if ((x>0)&&(x<num_x)&&(y<num_y))
				AddVertex(pos + radius * vector(pi*(float(x)-(float)num_x/2.0f)/num_x,pi*2.0f* y/num_y,0).ang2dir());
		}
	// create new triangles
	for (int y=0;y<num_y;y++){
		Array<int> v;
		v.add(0);
		v.add(2 +  y);
		v.add(2 + (y+1)%num_y);
		Array<vector> sv;
		sv.add(ball_ang(0, y+1));
		sv.add(ball_ang(1, y));
		sv.add(ball_ang(1, y+1));
		AddPolygonSingleTexture(v, sv);
	}
	for (int y=0;y<num_y;y++){
		Array<int> v;
		v.add(2+num_y*(num_x-2)+y);
		v.add(1);
		v.add(2+num_y*(num_x-2)+(y+1)%num_y);
		Array<vector> sv;
		sv.add(ball_ang(num_x - 1, y));
		sv.add(ball_ang(num_x, y));
		sv.add(ball_ang(num_x - 1, y+1));
		AddPolygonSingleTexture(v, sv);
	}
	for (int x=1;x<num_x-1;x++)
		for (int y=0;y<num_y;y++){
			Array<int> v;
			v.add(2 + num_y *(x-1) +(y+1)%num_y);
			v.add(2 + num_y *(x-1) + y);
			v.add(2 + num_y * x    + y);
			v.add(2 + num_y * x    + (y+1)%num_y);
			Array<vector> sv;
			sv.add(ball_ang(x  , y+1));
			sv.add(ball_ang(x  , y  ));
			sv.add(ball_ang(x+1, y  ));
			sv.add(ball_ang(x+1, y+1));
			AddPolygonSingleTexture(v, sv);
		}
}

void ModelGeometryBall::__init__(const vector &pos, float radius, int num_x, int num_y)
{
	new(this) ModelGeometryBall(pos, radius, num_x, num_y);
}

