/*
 * ModelSelection.cpp
 *
 *  Created on: 13.02.2020
 *      Author: michi
 */

#include "ModelSelection.h"
#include "ModelPolygon.h"
#include "ModelMesh.h"


void ModelSelection::clear() {
	vertex.clear();
	polygon.clear();
	edge.clear();
	bone.clear();
}

bool ModelSelection::consistent_surfaces(ModelMesh *m) const {
	for (int ei: edge) {
		auto &e = m->edge[ei];
		for (int k=0; k<2; k++)
			if ((e.polygon[k] >= 0) and !polygon.contains(e.polygon[k]))
				return false;
	}
	return true;
}

void ModelSelection::expand_to_surfaces(ModelMesh *m) {
	while (true) {
		bool changed = false;
		for (auto &e: m->edge)
			if (vertex.contains(e.vertex[0]) != vertex.contains(e.vertex[1])) {
				vertex.add(e.vertex[0]);
				vertex.add(e.vertex[1]);
				changed = true;
			}
		if (!changed)
			break;
	}
	foreachi (auto &p, m->polygon, i)
		for (int k=0; k<p.side.num; k++)
			if (vertex.contains(p.side[k].vertex))
				polygon.add(i);
}

ModelSelection ModelSelection::all(ModelMesh *m) {
	ModelSelection r;
	for (int i=0; i<m->vertex.num; i++)
		r.vertex.add(i);
	for (int i=0; i<m->polygon.num; i++)
		r.polygon.add(i);
	for (int i=0; i<m->edge.num; i++)
		r.edge.add(i);
	return r;
}

