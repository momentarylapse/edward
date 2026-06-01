//
// Created by michi on 18.05.25.
//

#ifndef MESHEXTRUDEPOLYGONS_H
#define MESHEXTRUDEPOLYGONS_H

#include <lib/history/Data.h>
#include <stuff/Selection.h>

class PolygonMesh;
class MeshEdit;

MeshEdit mesh_prepare_extrude_polygons(const PolygonMesh& mesh, const Selection& sel, float distance, bool keep_connected);



#endif //MESHEXTRUDEPOLYGONS_H
