/*
 * ActionModelBrushExtrude.cpp
 *
 *  Created on: 22.08.2013
 *      Author: michi
 */

#include "ActionModelBrushExtrude.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelBrushExtrude::ActionModelBrushExtrude(const vector &_pos, const vector &_n, float _radius, float _depth)
{
	pos = _pos;
	n = _n;
	radius = _radius;
	depth = _depth;
}

ActionModelBrushExtrude::~ActionModelBrushExtrude()
{
}

void* ActionModelBrushExtrude::execute(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	float r2 = radius * radius;

	for (int i=0;i<m->vertex.num;i++){
		float d2 = (pos - m->vertex[i].pos).length_sqr();
		if (d2 < r2 * 2){
			index.add(i);
			pos_old.add(m->vertex[i].pos);
			m->vertex[i].pos += n * depth * exp(- d2 / r2 * 2);
		}
	}
	m->setNormalsDirtyByVertices(index);


	return NULL;
}

void ActionModelBrushExtrude::undo(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->setNormalsDirtyByVertices(index);
	foreachi(int i, index, ii)
		m->vertex[i].pos = pos_old[ii];

	index.clear();
	pos_old.clear();
}
