//
// Created by michi on 18.05.25.
//

#ifndef MESHEXTRUDEPOLYGONS_H
#define MESHEXTRUDEPOLYGONS_H

#include <data/Data.h>

class PolygonMesh;
class MeshEdit;

MeshEdit mesh_prepare_extrude_polygons(const PolygonMesh& mesh, const Data::Selection& sel, float distance, bool keep_connected);



#endif //MESHEXTRUDEPOLYGONS_H
