/*
 * ActionModelBrushSmooth.cpp
 *
 *  Created on: 31.08.2013
 *      Author: michi
 */

#include "ActionModelBrushSmooth.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelBrushSmooth::ActionModelBrushSmooth(const vector &_pos, const vector &_n, float _radius)
{
	pos = _pos;
	n = _n;
	radius = _radius;
}

ActionModelBrushSmooth::~ActionModelBrushSmooth()
{
}

void* ActionModelBrushSmooth::execute(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	float r2 = radius * radius;

	for (int i=0;i<m->vertex.num;i++){
		float d2 = (pos - m->vertex[i].pos).length_sqr();
		if (d2 < r2 * 2){
			index.add(i);
			pos_old.add(m->vertex[i].pos);
			vector d = (m->vertex[i].pos - pos);
			d = d - (d * n) * n * exp(- d2 / r2 * 2);
			m->vertex[i].pos = pos + d;
		}
	}
	m->setNormalsDirtyByVertices(index);


	return NULL;
}

void ActionModelBrushSmooth::undo(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->setNormalsDirtyByVertices(index);
	foreachi(int i, index, ii)
		m->vertex[i].pos = pos_old[ii];

	index.clear();
	pos_old.clear();
}
