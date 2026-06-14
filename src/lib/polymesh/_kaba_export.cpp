#include "_kaba_export.h"
#include <lib/base/base.h>
#include <lib/math/Box.h>
#include "MeshEdit.h"
#include "../kapi/KabaExporter.h"
#include "Mesh.h"
#include "create/Ball.h"
#include "create/Cube.h"
#include "create/Cylinder.h"
#include "create/Plane.h"
#include "create/Platonic.h"
#include "create/Sphere.h"
#include "create/Teapot.h"
#include "create/Torus.h"
#include "create/TorusKnot.h"
#include "edit/AutoMergePolygons.h"
#include "edit/BevelEdges.h"
#include "edit/ExtrudePolygons.h"
#include "edit/InvertPolygons.h"

using namespace polymesh;


base::set<int> setify(const Array<int>& _sel) {
	base::set<int> sel;
	for (int i: _sel)
		sel.add(i);
	return sel;
}


MeshEdit kaba_wrap_extrude_polygons(const Mesh& mesh, const Array<int>& sel, float distance, bool keep_connected) {
	return polymesh::extrude_polygons(mesh, setify(sel), distance, keep_connected);
}

MeshEdit kaba_wrap_invert_polygons(const Mesh& mesh, const Array<int>& sel) {
	return polymesh::invert_polygons(mesh, setify(sel));
}

MeshEdit kaba_wrap_auto_merge_polygons(const Mesh& mesh, const Array<int>& sel, float dang) {
	return polymesh::auto_merge_polygons(mesh, setify(sel), dang);
}

MeshEdit kaba_wrap_bevel_edges(const Mesh& mesh, const Array<int>& selv, const Array<int>& sele, float radius) {
	return polymesh::bevel_edges(mesh, setify(selv), setify(sele), radius);
}

void export_package_polymesh(kaba::IExporter* ext) {
	ext->package_info("polymesh", "0.3");

	ext->declare_class_size("PMVertex", sizeof(Vertex));
	ext->declare_class_element("PMVertex.pos", &Vertex::pos);
	ext->declare_class_element("PMVertex.normal_mode", &Vertex::normal_mode);
	ext->declare_class_element("PMVertex.bone_index", &Vertex::bone_index);
	ext->declare_class_element("PMVertex.normal_dirty", &Vertex::normal_dirty);
	ext->declare_class_element("PMVertex.smoothing_id", &Vertex::smoothing_id);
	ext->link_class_func("PMVertex.__init__:PMVertex", &kaba::generic_init<Vertex>);
	ext->link_class_func("PMVertex.__init__:PMVertex:math.vec3", &kaba::generic_init_ext<Vertex, const vec3&>);

	ext->declare_class_size("Polygon.Side", sizeof(PolygonSide));
	ext->declare_class_element("Polygon.Side.vertex", &PolygonSide::vertex);
	ext->declare_class_element("Polygon.Side.uv", &PolygonSide::uv);
	ext->declare_class_element("Polygon.Side.normal_index", &PolygonSide::normal_index);
	ext->declare_class_element("Polygon.Side.normal", &PolygonSide::normal);
	ext->declare_class_element("Polygon.Side.triangulation", &PolygonSide::triangulation);

	ext->declare_class_size("Polygon", sizeof(Polygon));
	ext->declare_class_element("Polygon.sides", &Polygon::side);
	ext->declare_class_element("Polygon.temp_normal", &Polygon::temp_normal);
	ext->declare_class_element("Polygon.normal_dirty", &Polygon::normal_dirty);
	ext->declare_class_element("Polygon.triangulation_dirty", &Polygon::triangulation_dirty);
	ext->declare_class_element("Polygon.material", &Polygon::material);
	ext->link_class_func("Polygon.get_vertices", &Polygon::get_vertices);

	ext->declare_class_size("Mesh", sizeof(Mesh));
	ext->link_class_func("Mesh.__init__", &kaba::generic_init<Mesh>);
	ext->link_class_func("Mesh.__delete__", &kaba::generic_delete<Mesh>);
	ext->link_class_func("Mesh.__assign__", &kaba::generic_assign<Mesh>);
	ext->link_class_func("Mesh.add_vertex", &Mesh::add_vertex);
	ext->link_class_func("Mesh.add_polygon", &Mesh::add_polygon);
	ext->link_class_func("Mesh.add_polygon_auto_texture", &Mesh::add_polygon_auto_texture);
	ext->link_class_func("Mesh.bounding_box", &Mesh::bounding_box);
	ext->link_class_func("Mesh.add", &Mesh::add);
	ext->link_class_func("Mesh.smoothen", &Mesh::smoothen);
	ext->link_class_func("Mesh.update_normals", &Mesh::update_normals);
	ext->link_class_func("Mesh.transform", &Mesh::transform);
	ext->link_class_func("Mesh.invert", &Mesh::invert);
	ext->link_class_func("Mesh.build", &Mesh::build_x);
	ext->declare_class_element("Mesh.vertices", &Mesh::vertices);
	ext->declare_class_element("Mesh.polygons", &Mesh::polygons);
	ext->declare_class_element("Mesh.spheres", &Mesh::spheres);
	ext->declare_class_element("Mesh.cylinders", &Mesh::cylinders);

	ext->declare_class_size("MeshEdit", sizeof(MeshEdit));
	ext->declare_class_element("MeshEdit._del_vertices", &MeshEdit::_del_vertices);
	ext->declare_class_element("MeshEdit._del_polygons", &MeshEdit::_del_polygons);
	ext->declare_class_element("MeshEdit._new_vertices", &MeshEdit::_new_vertices);
	ext->declare_class_element("MeshEdit._new_polygons", &MeshEdit::_new_polygons);
	ext->link_class_func("MeshEdit.__init__", &kaba::generic_init<MeshEdit>);
	ext->link_class_func("MeshEdit.__delete__", &kaba::generic_delete<MeshEdit>);
	ext->link_class_func("MeshEdit.__assign__", &kaba::generic_assign<MeshEdit>);
	ext->link_class_func("MeshEdit.delete_vertex", &MeshEdit::delete_vertex);
	ext->link_class_func("MeshEdit.delete_polygon", &MeshEdit::delete_polygon);
	ext->link_class_func("MeshEdit.add_vertex", &MeshEdit::add_vertex);
	ext->link_class_func("MeshEdit.add_polygon", &MeshEdit::add_polygon);
	ext->link_class_func("MeshEdit.apply", &MeshEdit::apply);
	ext->link_class_func("MeshEdit.apply_inplace", &MeshEdit::apply_inplace);

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

	ext->link_func("bevel_edges", &kaba_wrap_bevel_edges);
	ext->link_func("extrude_polygons", &kaba_wrap_extrude_polygons);
	ext->link_func("invert_polygons", &kaba_wrap_invert_polygons);
	ext->link_func("auto_merge_polygons", &kaba_wrap_auto_merge_polygons);
}


