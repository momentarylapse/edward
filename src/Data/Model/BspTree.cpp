/*
 * BspTree.cpp
 *
 *  Created on: 23.10.2013
 *      Author: michi
 */

#include "BspTree.h"
#include "DataModel.h"

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
	void add(ModelPolygon &p, DataModel *m, const plane &_pl, float epsilon)
	{
		bool some_in = false;
		bool some_out = false;
		for (int j=0;j<p.Side.num;j++){
			float d = pl.distance(m->Vertex[p.Side[j].Vertex].pos);
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

void BspTree::add(ModelPolygon &p, DataModel *m, float epsilon)
{
	plane pl;
	PlaneFromPointNormal(pl, m->Vertex[p.Side[0].Vertex].pos, p.TempNormal);
	if (b)
		b->add(p, m, pl, epsilon);
	else
		b = new BspBranch(pl);
}

bool BspTree::inside(const vector &v)
{
	BspBranch *bb = b;
	bool _inside = false;
	while(bb){
		_inside = (bb->pl.distance(v) < 0);
		bb = _inside ? bb->i : bb->o;
	}
	return _inside;
}
