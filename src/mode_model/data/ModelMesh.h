/*
 * ModelMesh.h
 *
 *  Created on: 13.02.2020
 *      Author: michi
 */

#ifndef SRC_DATA_MODEL_MODELMESH_H_
#define SRC_DATA_MODEL_MODELMESH_H_

#include <view/SingleData.h>
#include <data/mesh/PolygonMesh.h>
#include <lib/math/vec4.h>

struct Box;
class DataModel;
class ModelTriangleMesh;
class ModelSelection;
class MeshInsideTestData;


// only for use in MultiView...
struct ModelSkinVertexDummy: multiview::SingleData {
};

struct ModelBall: multiview::SingleData {
	int index;
	float radius;

	/*float hover_distance(MultiView::Window *win, const vec2 &m, vec3 &tp, float &z) override;
	bool in_rect(MultiView::Window *win, const rect &r) override;
	bool overlap_rect(MultiView::Window *win, const rect &r) override;*/
};

struct ModelCylinder: multiview::SingleData {
	int index[2];
	float radius;
	bool round;

	/*float hover_distance(MultiView::Window *win, const vec2 &m, vec3 &tp, float &z) override;
	bool in_rect(MultiView::Window *win, const rect &r) override;
	bool overlap_rect(MultiView::Window *win, const rect &r) override;*/
};






struct ModelMesh : PolygonMesh {
	explicit ModelMesh(DataModel *model);
	~ModelMesh();

	DataModel *model;

	void clear();

	// low level (un-action'ed)
	void add_vertex(const vec3 &pos, const ivec4 &bone, const vec4 &weight, int normal_mode, int index = -1);
	void remove_lonely_vertex(int v);
	void _shift_vertex_links(int offset, int delta);
	void _add_polygon(const Array<int> &v, int material, const Array<vec3> &sv, int index = -1);
	void _remove_polygon(int index);
	void _add_vertices(const Array<MeshVertex> &vertices);
	void _post_vertex_number_change_update();

	bool test_sanity(const string &loc);

	void build_topology();

	MeshInsideTestData *inside_data;
	/*bool is_inside(const vec3 &p);
	void begin_inside_tests();
	bool inside_test(const vec3 &p);
	void end_inside_tests();
	Array<int> get_boundary_loop(int v0);
	Array<Array<int> > get_connected_components();*/


	Array<ModelSkinVertexDummy> skin_vertex; // only temporary...

	// purely physical
	Array<ModelBall> ball;
	Array<ModelCylinder> cylinder;



	void on_post_action_update();
	void import_from_triangle_skin(int index);
	void export_to_triangle_mesh(ModelTriangleMesh &trias);
	Box bounding_box();
	void set_normals_dirty_by_vertices(const Array<int> &index);
	void set_all_normals_dirty();
	void update_normals();
	void clear_selection();
	void selection_from_polygons();
	void selection_from_vertices();

	ModelSelection get_selection() const;
	void set_selection(const ModelSelection &s);

	void set_show_vertices(Array<MeshVertex> &vert);
	Array<MeshVertex> show_vertices;


	PolygonMesh copy_geometry() const;
};

#endif /* SRC_DATA_MODEL_MODELMESH_H_ */
