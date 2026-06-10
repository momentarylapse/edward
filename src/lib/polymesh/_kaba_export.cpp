#include "_kaba_export.h"
#include "../base/base.h"
#include <lib/math/Box.h>
#include "../kapi/KabaExporter.h"
#include "PolygonMesh.h"
#include "create/Ball.h"
#include "create/Cube.h"
#include "create/Cylinder.h"
#include "create/Plane.h"
#include "create/Platonic.h"
#include "create/Sphere.h"
#include "create/Teapot.h"
#include "create/Torus.h"
#include "create/TorusKnot.h"

void export_package_polymesh(kaba::IExporter* ext) {
	ext->package_info("polymesh", "0.1");

	ext->declare_class_size("PMVertex", sizeof(MeshVertex));
	ext->declare_class_element("PMVertex.pos", &MeshVertex::pos);
	ext->declare_class_element("PMVertex.normal_mode", &MeshVertex::normal_mode);
	ext->declare_class_element("PMVertex.bone_index", &MeshVertex::bone_index);
	ext->declare_class_element("PMVertex.normal_dirty", &MeshVertex::normal_dirty);
	ext->declare_class_element("PMVertex.ref_count", &MeshVertex::ref_count);

	ext->declare_class_size("Polygon.Side", sizeof(PolygonSide));
	ext->declare_class_element("Polygon.Side.vertex", &PolygonSide::vertex);
	ext->declare_class_element("Polygon.Side.uv", &PolygonSide::skin_vertex);
	ext->declare_class_element("Polygon.Side.normal_index", &PolygonSide::normal_index);
	ext->declare_class_element("Polygon.Side.normal", &PolygonSide::normal);
	ext->declare_class_element("Polygon.Side.triangulation", &PolygonSide::triangulation);

	ext->declare_class_size("Polygon", sizeof(Polygon));
	ext->declare_class_element("Polygon.sides", &Polygon::side);
	ext->declare_class_element("Polygon.temp_normal", &Polygon::temp_normal);
	ext->declare_class_element("Polygon.normal_dirty", &Polygon::normal_dirty);
	ext->declare_class_element("Polygon.triangulation_dirty", &Polygon::triangulation_dirty);
	ext->declare_class_element("Polygon.material", &Polygon::material);

	ext->declare_class_size("PolygonMesh", sizeof(PolygonMesh));
	ext->link_class_func("PolygonMesh.__init__", &kaba::generic_init<PolygonMesh>);
	ext->link_class_func("PolygonMesh.__delete__", &kaba::generic_delete<PolygonMesh>);
	ext->link_class_func("PolygonMesh.__assign__", &kaba::generic_assign<PolygonMesh>);
	ext->link_class_func("PolygonMesh.bounding_box", &PolygonMesh::bounding_box);
	ext->link_class_func("PolygonMesh.add", &PolygonMesh::add);
	ext->link_class_func("PolygonMesh.smoothen", &PolygonMesh::smoothen);
	ext->link_class_func("PolygonMesh.transform", &PolygonMesh::transform);
	ext->link_class_func("PolygonMesh.invert", &PolygonMesh::invert);
	ext->link_class_func("PolygonMesh.build", &PolygonMesh::build_x);
	ext->declare_class_element("PolygonMesh.vertices", &PolygonMesh::vertices);
	ext->declare_class_element("PolygonMesh.polygons", &PolygonMesh::polygons);
	ext->declare_class_element("PolygonMesh.spheres", &PolygonMesh::spheres);
	ext->declare_class_element("PolygonMesh.cylinders", &PolygonMesh::cylinders);

	ext->link_func("create_ball", &polymesh::create_ball);
	ext->link_func("create_cube", &polymesh::create_cube);
//	ext->link_func("create_cylinder", &polymesh::create_cylinder);
	ext->link_func("create_plane", &polymesh::create_plane);
	ext->link_func("create_platonic", &polymesh::create_platonic);
	ext->link_func("create_tetrahedron", &polymesh::create_tetrahedron);
	ext->link_func("create_octahedron", &polymesh::create_octahedron);
	ext->link_func("create_dodecahedron", &polymesh::create_dodecahedron);
	ext->link_func("create_icosahedron", &polymesh::create_icosahedron);
	ext->link_func("create_sphere", &polymesh::create_sphere);
	ext->link_func("create_teapot", &polymesh::create_teapot);
	ext->link_func("create_torus", &polymesh::create_torus);
	ext->link_func("create_torus_knot", &polymesh::create_torus_knot);
}


