//
// Created by michi on 18.05.25.
//

#ifndef MESHBEVELEDGES_H
#define MESHBEVELEDGES_H


#include <lib/history/Data.h>
#include <stuff/Selection.h>

class PolygonMesh;
class MeshEdit;

MeshEdit mesh_prepare_bevel_edges(const PolygonMesh& mesh, const Selection& sel, float distance);



#endif //MESHBEVELEDGES_H
