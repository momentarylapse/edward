/*
 * ActionModelAddBall.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionModelAddBall.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Polygon/ActionModelAddPolygonSingleTexture.h"
#include "../Surface/ActionModelSurfaceAutoWeld.h"
#include "../../../../Data/Model/DataModel.h"


#define ball_ang(x, y)	vector((float)(y)/(float)num_y,(float)(x)/(float)num_x,0)

ActionModelAddBall::ActionModelAddBall(DataModel *m, const vector &_pos, float _radius, int _num_x, int _num_y)
{
	pos = _pos;
	radius = _radius;
	num_x = _num_x;
	num_y = _num_y;
}

void *ActionModelAddBall::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	int nv = m->Vertex.num;
	int material = m->CurrentMaterial;

	// ball from disks

	// create new vertices
	AddSubAction(new ActionModelAddVertex(pos + e_y * radius), m);
	AddSubAction(new ActionModelAddVertex(pos - e_y * radius), m);
	for (int x=0;x<num_x+1;x++)
		for (int y=0;y<num_y+1;y++){
			if ((x>0)&&(x<num_x)&&(y<num_y))
				AddSubAction(new ActionModelAddVertex(pos + radius * vector(pi*(float(x)-(float)num_x/2.0f)/num_x,pi*2.0f* y/num_y,0).ang2dir()), m);
		}
	// create new triangles
	for (int y=0;y<num_y;y++){
		Array<int> v;
		v.add(nv);
		v.add(nv + 2 +  y);
		v.add(nv + 2 + (y+1)%num_y);
		Array<vector> sv;
		sv.add(ball_ang(0, y+1));
		sv.add(ball_ang(1, y));
		sv.add(ball_ang(1, y+1));
		AddSubAction(new ActionModelAddPolygonSingleTexture(v, material, sv), m);
	}
	for (int y=0;y<num_y;y++){
		Array<int> v;
		v.add(nv+2+num_y*(num_x-2)+y);
		v.add(nv + 1);
		v.add(nv+2+num_y*(num_x-2)+(y+1)%num_y);
		Array<vector> sv;
		sv.add(ball_ang(num_x - 1, y));
		sv.add(ball_ang(num_x, y));
		sv.add(ball_ang(num_x - 1, y+1));
		AddSubAction(new ActionModelAddPolygonSingleTexture(v, material, sv), m);
	}
	for (int x=1;x<num_x-1;x++)
		for (int y=0;y<num_y;y++){
			Array<int> v;
			v.add(nv+2 + num_y *(x-1) +(y+1)%num_y);
			v.add(nv+2 + num_y *(x-1) + y);
			v.add(nv+2 + num_y * x    + y);
			v.add(nv+2 + num_y * x    + (y+1)%num_y);
			Array<vector> sv;
			sv.add(ball_ang(x  , y+1));
			sv.add(ball_ang(x  , y  ));
			sv.add(ball_ang(x+1, y  ));
			sv.add(ball_ang(x+1, y+1));
			AddSubAction(new ActionModelAddPolygonSingleTexture(v, material, sv), m);
		}
	return &m->Surface.back();
}
