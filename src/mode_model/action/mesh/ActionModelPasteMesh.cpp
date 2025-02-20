/*
 * ActionModelPasteMesh.cpp
 *
 *  Created on: 07.06.2012
 *      Author: michi
 */

#include "ActionModelPasteMesh.h"
#include "../../data/DataModel.h"
#include "../../data/ModelMesh.h"

ActionModelPasteMesh::ActionModelPasteMesh(const PolygonMesh &_geo, int _default_material) :
	geo(_geo)
{
	default_material = _default_material;
}

void *ActionModelPasteMesh::execute(Data *d) {
	auto m = dynamic_cast<DataModel*>(d);
	//m->clearSelection();

	int nv = m->mesh->vertices.num;

	m->mesh->_add_vertices(geo.vertices);

	for (auto &t: geo.polygons) {
		Array<int> v;
		for (int k=0;k<t.side.num;k++)
			v.add(nv + t.side[k].vertex);
		Array<vec3> sv;
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
			for (int k=0;k<t.side.num;k++)
				sv.add(t.side[k].skin_vertex[l]);
		int mat = (t.material >= 0) ? t.material : default_material;

		m->mesh->_add_polygon(v, mat, sv);
	}
	return nullptr;
}

void ActionModelPasteMesh::undo(Data *d) {
	auto m = dynamic_cast<DataModel*>(d);
	//m->clear_selection();

	m->mesh->vertices.resize(m->mesh->vertices.num - geo.vertices.num);
	m->mesh->_post_vertex_number_change_update();
	m->mesh->polygons.resize(m->mesh->polygons.num - geo.polygons.num);
}
