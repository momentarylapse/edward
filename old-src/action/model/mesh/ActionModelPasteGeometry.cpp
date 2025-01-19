/*
 * ActionModelPasteGeometry.cpp
 *
 *  Created on: 07.06.2012
 *      Author: michi
 */

#include "ActionModelPasteGeometry.h"
#include "vertex/ActionModelAddVertex.h"
#include "polygon/ActionModelAddPolygon.h"
#include "../../../data/model/DataModel.h"
#include "../../../data/model/ModelMesh.h"
#include "../../../data/model/geometry/Geometry.h"
#include "../../../y/world/components/Animator.h"

ActionModelPasteGeometry::ActionModelPasteGeometry(const Geometry &_geo, int _default_material) :
	geo(_geo)
{
	default_material = _default_material;
	num_edges_before = 0;
}

void *ActionModelPasteGeometry::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->clearSelection();

	int nv = m->edit_mesh->vertex.num;
	num_edges_before = m->edit_mesh->edge.num;

	m->edit_mesh->_add_vertices(geo.vertex);

	for (ModelPolygon &t: geo.polygon) {
		Array<int> v;
		for (int k=0;k<t.side.num;k++)
			v.add(nv + t.side[k].vertex);
		Array<vec3> sv;
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
			for (int k=0;k<t.side.num;k++)
				sv.add(t.side[k].skin_vertex[l]);
		int mat = (t.material >= 0) ? t.material : default_material;

		m->edit_mesh->_add_polygon(v, mat, sv);
	}
	return nullptr;
}

void ActionModelPasteGeometry::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->clearSelection();

	m->edit_mesh->vertex.resize(m->edit_mesh->vertex.num - geo.vertex.num);
	m->edit_mesh->_post_vertex_number_change_update();
	m->edit_mesh->polygon.resize(m->edit_mesh->polygon.num - geo.polygon.num);
	m->edit_mesh->edge.resize(num_edges_before);

}
