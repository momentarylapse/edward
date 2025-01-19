/*
 * BspTree.cpp
 *
 *  Created on: 23.10.2013
 *      Author: michi
 */

#include "BspTree.h"
#include "DataModel.h"
#include "ModelPolygon.h"
#include "ModelMesh.h"

struct BspBranch
{
	plane pl;
	BspBranch *i, *o;

	BspBranch(const plane &_pl)
	{
		i = NULL;
		o = NULL;
		pl = _pl;
	}
	~BspBranch()
	{
		if (i)
			delete(i);
		if (o)
			delete(o);
	}
	void add(ModelPolygon &p, ModelMesh *m, const plane &_pl, float epsilon)
	{
		bool some_in = false;
		bool some_out = false;
		for (int j=0;j<p.side.num;j++){
			float d = pl.distance(m->vertex[p.side[j].vertex].pos);
			if (d > epsilon)
				some_out = true;
			else if (d < - epsilon)
				some_in = true;
		}

		if (some_in){
			if (i)
				i->add(p, m, _pl, epsilon);
			else
				i = new BspBranch(_pl);
		}
		if (some_out){
			if (o)
				o->add(p, m, _pl, epsilon);
			else
				o = new BspBranch(_pl);
		}
	}
};

BspTree::BspTree()
{
	b = NULL;
}

BspTree::~BspTree()
{
	if (b)
		delete(b);
}

void BspTree::add(ModelPolygon &p, ModelMesh *m, float epsilon)
{
	plane pl;
	pl = plane::from_point_normal( m->vertex[p.side[0].vertex].pos, p.temp_normal);
	if (b)
		b->add(p, m, pl, epsilon);
	else
		b = new BspBranch(pl);
}

bool BspTree::inside(const vec3 &v)
{
	BspBranch *bb = b;
	bool _inside = false;
	while(bb){
		_inside = (bb->pl.distance(v) < 0);
		bb = _inside ? bb->i : bb->o;
	}
	return _inside;
}

