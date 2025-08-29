/*
 * ModelMesh.h
 *
 *  Created on: 13.02.2020
 *      Author: michi
 */

#ifndef SRC_DATA_MODEL_MODELMESH_H_
#define SRC_DATA_MODEL_MODELMESH_H_

#include <view/SingleData.h>
#include <lib/mesh/PolygonMesh.h>
#include <lib/math/vec4.h>
#include "data/Data.h"

struct Box;
class DataModel;
class ModelTriangleMesh;
class ModelSelection;
class MeshInsideTestData;


// only for use in MultiView...
struct ModelSkinVertexDummy: multiview::SingleData {
};






struct ModelMesh : PolygonMesh {
	explicit ModelMesh();
	~ModelMesh();

	// low level (un-action'ed)
	void add_vertex(const vec3 &pos, const ivec4 &bone, const vec4 &weight, int normal_mode, int index = -1);
	void remove_lonely_vertex(int v);
	void _shift_vertex_links(int offset, int delta);
	void _add_polygon(const Array<int> &v, int material, const Array<vec3> &sv, int index = -1);
	void _remove_polygon(int index);
	void _add_vertices(const Array<MeshVertex> &vertices, DataModel* model);
	void _post_vertex_number_change_update(DataModel* model);


	void build_topology();

	MeshInsideTestData *inside_data;
	/*bool is_inside(const vec3 &p);
	void begin_inside_tests();
	bool inside_test(const vec3 &p);
	void end_inside_tests();
	Array<int> get_boundary_loop(int v0);
	Array<Array<int> > get_connected_components();*/





	void on_post_action_update();
	void import_from_triangle_skin(int index);
	void export_to_triangle_mesh(ModelTriangleMesh &trias, DataModel* model);
	void set_normals_dirty_by_vertices(const Array<int> &index);
	void set_all_normals_dirty();
	void update_normals();
	void clear_selection();
	void selection_from_polygons();
	void selection_from_vertices();

	ModelSelection get_selection() const;
	void set_selection(const ModelSelection &s);


	PolygonMesh copy_geometry(const Data::Selection& sel) const;
};

#endif /* SRC_DATA_MODEL_MODELMESH_H_ */
