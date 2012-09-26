/*
 * ActionModelAddPolygonAutoSkin.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ActionModelAddPolygonAutoSkin.h"
#include "ActionModelAddTriangleWithSkinGenerator.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/SkinGenerator.h"

ActionModelAddPolygonAutoSkin::ActionModelAddPolygonAutoSkin(Array<int> &_vertex)
{
	vertex = _vertex;
}


static vector get_cloud_normal(DataModel *m, const Array<int> &v)
{
	Array<vector> p;
	for (int i=1;i<v.num;i++){
		p.add(m->Vertex[v[i]].pos - m->Vertex[v[0]].pos);
		p.back().normalize();
	}
	for (int i=0;i<p.num;i++)
		for (int j=i+1;j<p.num;j++){
			vector d = (p[i] ^ p[j]);
			float l = d.length();
			if (l > 0.1f)
				return d / l;
		}
	return v_0;
}

static void init_skin_generator(DataModel *m, Array<int> &v, SkinGenerator &sg)
{
	vector n = get_cloud_normal(m, v);
	vector d[2];
	d[0] = n.ortho();
	d[1] = d[0] ^ n;
	float boundary[2][2], l[2];
	for (int k=0;k<2;k++){
		boundary[k][0] = boundary[k][1] = d[k] * m->Vertex[v[0]].pos;
		foreach(int vi, v){
			float f = d[k] * m->Vertex[vi].pos;
			if (f < boundary[k][0])
				boundary[k][0] = f;
			if (f > boundary[k][1])
				boundary[k][1] = f;
		}
		l[k] = (boundary[k][1] - boundary[k][0]);
	}
	sg.init_affine(d[0] / l[0], - boundary[0][0] / l[0], d[1] / l[1], - boundary[1][0] / l[1]);

}

void *ActionModelAddPolygonAutoSkin::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	SkinGenerator sg;
	init_skin_generator(m, vertex, sg);

	return AddSubAction(new ActionModelAddTriangleWithSkinGenerator(m, vertex, m->CurrentMaterial, sg), m);
}

