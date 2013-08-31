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

	for (int i=0;i<m->Vertex.num;i++){
		float d2 = (pos - m->Vertex[i].pos).length_sqr();
		if (d2 < r2 * 2){
			index.add(i);
			pos_old.add(m->Vertex[i].pos);
			vector d = (m->Vertex[i].pos - pos);
			d = d - (d * n) * n * exp(- d2 / r2 * 2);
			m->Vertex[i].pos = pos + d;
		}
	}
	m->SetNormalsDirtyByVertices(index);


	return NULL;
}

void ActionModelBrushSmooth::undo(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->SetNormalsDirtyByVertices(index);
	foreachi(int i, index, ii)
		m->Vertex[i].pos = pos_old[ii];

	index.clear();
	pos_old.clear();
}
