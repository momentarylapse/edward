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

void ModelSelection::set(const ModelEdge &e) {
	if (has(e))
		return;
	Edge ee;
	ee.a = min(e.vertex[0], e.vertex[1]);
	ee.b = max(e.vertex[0], e.vertex[1]);
	edge.add(ee);
}

bool ModelSelection::has(const ModelEdge &e) const {
	Edge ee;
	ee.a = min(e.vertex[0], e.vertex[1]);
	ee.b = max(e.vertex[0], e.vertex[1]);
	for (auto &eee: edge)
		if (eee.a == ee.a and eee.b == ee.b)
			return true;
	return false;
}



bool ModelSelection::consistent_surfaces(ModelMesh *m) const {
	for (auto &e: m->edge)
		if (has(e)) {
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

ModelSelection ModelSelection::filter_view_stage(ModelMesh *m, int view_stage) const {
	ModelSelection r;
	for (int i: vertex)
		if (m->vertex[i].view_stage >= view_stage)
			r.vertex.add(i);
	for (int i: polygon)
		if (m->polygon[i].view_stage >= view_stage)
			r.polygon.add(i);
	//for (auto &e: m->edge)
	//	r.set(e);
	return r;
}

ModelSelection ModelSelection::all(ModelMesh *m) {
	ModelSelection r;
	for (int i=0; i<m->vertex.num; i++)
		r.vertex.add(i);
	for (int i=0; i<m->polygon.num; i++)
		r.polygon.add(i);
	for (auto &e: m->edge)
		r.set(e);
	return r;
}

