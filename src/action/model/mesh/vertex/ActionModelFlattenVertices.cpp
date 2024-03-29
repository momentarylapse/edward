/*
 * ActionModelFlattenVertices.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ActionModelFlattenVertices.h"
#include "../../../../data/model/DataModel.h"
#include "../../../../data/model/ModelMesh.h"
#include <assert.h>

ActionModelFlattenVertices::ActionModelFlattenVertices(DataModel* m)
{
	foreachi(ModelVertex &v, m->edit_mesh->vertex, i)
		if (v.is_selected){
			index.add(i);
			old_pos.add(v.pos);
		}
}

ActionModelFlattenVertices::~ActionModelFlattenVertices()
{
}

vec3 VecCloudGetNormal(Array<vec3> &v)
{
	vec3 m = v_0;
	for (int i=0;i<v.num;i++)
		m += v[i];
	m /= v.num;

	mat3 I;
	memset(&I, 0, sizeof(I));

	for (vec3 &p: v){
		vec3 dp = p - m;
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


	vec3 amax = v_0;
	float lmax = 0;
	for (int i=-50;i<50;i++)
		for (int j=0;j<100;j++){
			vec3 a = vec3(i/100.0f*2*pi, j/100.0f*2*pi, 0).ang2dir();
			float l = vec3::dot(a, (I * a));
			if (l > lmax){
				lmax = l;
				amax = a;
			}
		}
	return amax;
}

void PlaneFromPointCloud(plane &pl, Array<vec3> &v)
{
	pl.n = VecCloudGetNormal(v);
	pl.d = 0;
	for (int i=0;i<v.num;i++)
		pl.d -= vec3::dot(pl.n, v[i]);
	pl.d /= v.num;
}

void* ActionModelFlattenVertices::execute(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	Array<vec3> cloud;
	plane pl;

	for (int i: index)
		cloud.add(m->edit_mesh->vertex[i].pos);
	PlaneFromPointCloud(pl, cloud);

	for (int i: index)
		m->edit_mesh->vertex[i].pos -= pl.distance(m->edit_mesh->vertex[i].pos) * pl.n;

	m->set_normals_dirty_by_vertices(index);
	return NULL;
}

void ActionModelFlattenVertices::undo(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreachi(int i, index, ii)
		m->edit_mesh->vertex[i].pos = old_pos[ii];

	m->set_normals_dirty_by_vertices(index);
}

bool ActionModelFlattenVertices::was_trivial()
{	return index.num == 0;	}


