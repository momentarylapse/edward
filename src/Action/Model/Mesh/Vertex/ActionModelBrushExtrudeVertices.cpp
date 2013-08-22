/*
 * ActionModelBrushExtrudeVertices.cpp
 *
 *  Created on: 22.08.2013
 *      Author: michi
 */

#include "ActionModelBrushExtrudeVertices.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelBrushExtrudeVertices::ActionModelBrushExtrudeVertices(const vector &_pos, const vector &_n, float _radius, float _depth)
{
	pos = _pos;
	n = _n;
	radius = _radius;
	depth = _depth;
}

ActionModelBrushExtrudeVertices::~ActionModelBrushExtrudeVertices()
{
}

void* ActionModelBrushExtrudeVertices::execute(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	float r2 = radius * radius;

	for (int i=0;i<m->Vertex.num;i++){
		float d2 = (pos - m->Vertex[i].pos).length_sqr();
		if (d2 < r2 * 2){
			index.add(i);
			pos_old.add(m->Vertex[i].pos);
			m->Vertex[i].pos += n * depth * exp(- d2 / r2 * 2);
		}
	}
	m->SetNormalsDirtyByVertices(index);


	return NULL;
}

void ActionModelBrushExtrudeVertices::undo(Data* d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	m->SetNormalsDirtyByVertices(index);
	foreachi(int i, index, ii)
		m->Vertex[i].pos = pos_old[ii];

	index.clear();
	pos_old.clear();
}
