//
// Created by michi on 18.05.25.
//

#ifndef MESHBEVELEDGES_H
#define MESHBEVELEDGES_H


#include <data/Data.h>

class PolygonMesh;
class MeshEdit;

MeshEdit mesh_prepare_bevel_edges(const PolygonMesh& mesh, const Data::Selection& sel, float distance);



#endif //MESHBEVELEDGES_H
