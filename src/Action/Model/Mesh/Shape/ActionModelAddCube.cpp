/*
 * ActionModelAddCube.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "ActionModelAddCube.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Polygon/ActionModelAddPolygonSingleTexture.h"
#include "../Surface/ActionModelSurfaceAutoWeld.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelAddCube::ActionModelAddCube(const vector &_pos, const vector &dv1, const vector &dv2, const vector &dv3, int num_1, int num_2, int num_3)
{
	pos = _pos;
	dv[0] = dv1;
	dv[1] = dv2;
	dv[2] = dv3;
	num[0] = num_1;
	num[1] = num_2;
	num[2] = num_3;

	// check orientation
	if ((dv[0] ^ dv[1]) * dv[2] < 0){
		pos += dv[2];
		dv[2] = - dv[2];
	}
}

ActionModelAddCube::~ActionModelAddCube()
{
}

void *ActionModelAddCube::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	int nv = m->Vertex.num;
	int material = m->CurrentMaterial;

	for (int f=0;f<6;f++){
	//	nv = Vertex.num;
		int nz = f % 3;
		int z = f / 3;
		int ny = (nz == 1) ? 2 : 1;
		int nx = 3 - ny - nz;

		// create new vertices
		for (int x=0;x<=num[nx];x++)
			for (int y=0;y<=num[ny];y++){
				int xx = ((nz == 2) ^ (z == 0)) ? x : num[nx] - x;
				vector dp = dv[nx] * (float)xx / (float)num[nx] + dv[ny] * (float)y / (float)num[ny] + dv[nz] * z;
				AddSubAction(new ActionModelAddVertex(pos + dp), m);
			}
		// create new triangles
		for (int x=0;x<num[nx];x++)
			for (int y=0;y<num[ny];y++){
				Array<int> v;
				v.add(nv + (num[ny]+1)* x   +y+1);
				v.add(nv + (num[ny]+1)* x   +y);
				v.add(nv + (num[ny]+1)*(x+1)+y);
				v.add(nv + (num[ny]+1)*(x+1)+y+1);
				Array<vector> sv;
				sv.add(vector((float) x   /(float)num[nx],(float)(y+1)/(float)num[ny],0));
				sv.add(vector((float) x   /(float)num[nx],(float) y   /(float)num[ny],0));
				sv.add(vector((float)(x+1)/(float)num[nx],(float) y   /(float)num[ny],0));
				sv.add(vector((float)(x+1)/(float)num[nx],(float)(y+1)/(float)num[ny],0));
				AddSubAction(new ActionModelAddPolygonSingleTexture(v, material, sv), m);
			}
		nv += (num[nx] + 1) * (num[ny] + 1);
	}

	float epsilon = min(min(dv[0].length() / (float)num[0], dv[1].length() / (float)num[1]), dv[2].length() / (float)num[2]) * 0.01f;

	// weld together
	for (int f=1;f<6;f++)
		AddSubAction(new ActionModelSurfaceAutoWeld(m->Surface.num - 2, m->Surface.num - 1, epsilon), m);

	return &m->Surface.back();
}


