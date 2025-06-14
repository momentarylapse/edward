/*
 * ActionModelPasteMesh.cpp
 *
 *  Created on: 07.06.2012
 *      Author: michi
 */

#include "ActionModelPasteMesh.h"
#include "../../data/DataModel.h"
#include "../../data/ModelMesh.h"

ActionModelPasteMesh::ActionModelPasteMesh(ModelMesh* m, const PolygonMesh &_geo, int _default_material) :
	geo(_geo)
{
	mesh = m;
	default_material = _default_material;
}

void *ActionModelPasteMesh::execute(Data* data) {
	auto d = dynamic_cast<DataModel*>(data);

	int nv = mesh->vertices.num;

	mesh->_add_vertices(geo.vertices, d);

	for (auto &t: geo.polygons) {
		Array<int> v;
		for (int k=0;k<t.side.num;k++)
			v.add(nv + t.side[k].vertex);
		Array<vec3> sv;
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
			for (int k=0;k<t.side.num;k++)
				sv.add(t.side[k].skin_vertex[l]);
		int mat = (t.material >= 0) ? t.material : default_material;

		mesh->_add_polygon(v, mat, sv);
	}
	d->out_topology_changed();
	return nullptr;
}

void ActionModelPasteMesh::undo(Data* data) {
	auto d = dynamic_cast<DataModel*>(data);

	mesh->vertices.resize(mesh->vertices.num - geo.vertices.num);
	mesh->_post_vertex_number_change_update(d);
	mesh->polygons.resize(mesh->polygons.num - geo.polygons.num);

	d->out_topology_changed();
}
