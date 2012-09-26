/*
 * ActionModelFlattenVertices.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ActionModelFlattenVertices.h"
#include "../../../../Data/Model/DataModel.h"
#include <assert.h>

ActionModelFlattenVertices::ActionModelFlattenVertices(DataModel* m)
{
	foreachi(ModelVertex &v, m->Vertex, i)
		if (v.is_selected){
			index.add(i);
			old_pos.add(v.pos);
		}
}

ActionModelFlattenVertices::~ActionModelFlattenVertices()
{
}

vector VecCloudGetNormal(Array<vector> &v)
{
	vector m = v_0;
	for (int i=0;i<v.num;i++)
		m += v[i];
	m /= v.num;

	matrix3 I;
	memset(&I, 0, sizeof(I));

	foreach(vector &p, v){
		vector dp = p - m;
		I._00 += dp.y*dp.y + dp.z*dp.z;
		I._11 += dp.z*dp.z + dp.x*dp.x;
		I._22 += dp.x*dp.x + dp.y*dp.y;
		I._01 -= dp.x*dp.y;
		I._02 -= dp.x*dp.z;
		I._12 -= dp.y*dp.z;
	}
	I._10 = I._01;
	I._20 = I._02;
	I._21 = I._12;


	vector amax = v_0;
	float lmax = 0;
	for (int i=-50;i<50;i++)
		for (int j=0;j<100;j++){
			vector a = vector(i/100.0f*2*pi, j/100.0f*2*pi, 0).ang2dir();
			float l = a * (I * a);
			if (l > lmax){
				lmax = l;
				amax = a;
			}
		}
	return amax;
}

void PlaneFromPointCloud(plane &pl, Array<vector> &v)
{
	pl.n = VecCloudGetNormal(v);
	pl.d = 0;
	for (int i=0;i<v.num;i++)
		pl.d -= pl.n * v[i];
	pl.d /= v.num;
}

void* ActionModelFlattenVertices::execute(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	Array<vector> cloud;
	plane pl;

	foreach(int i, index)
		cloud.add(m->Vertex[i].pos);
	PlaneFromPointCloud(pl, cloud);

	foreach(int i, index)
		m->Vertex[i].pos -= pl.distance(m->Vertex[i].pos) * pl.n;

	m->SetNormalsDirtyByVertices(index);
	return NULL;
}

void ActionModelFlattenVertices::undo(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreachi(int i, index, ii)
		m->Vertex[i].pos = old_pos[ii];

	m->SetNormalsDirtyByVertices(index);
}

