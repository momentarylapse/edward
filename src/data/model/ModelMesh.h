/*
 * ModelMesh.h
 *
 *  Created on: 13.02.2020
 *      Author: michi
 */

#ifndef SRC_DATA_MODEL_MODELMESH_H_
#define SRC_DATA_MODEL_MODELMESH_H_

#include "../../multiview/SingleData.h"
#include "../../lib/math/vec4.h"

class DataModel;
class ModelTriangleMesh;
class ModelPolygon;
class ModelSelection;
class MeshInsideTestData;
class Geometry;


class ModelVertex: public MultiView::SingleData {
public:
	int normal_mode;
	ivec4 bone_index;
	vec4 bone_weight;

	bool normal_dirty;
	int ref_count; // polygons

	ModelVertex();
	ModelVertex(const vec3 &pos);
};

class ModelEdge: public MultiView::SingleData {
public:
	//int NormalMode;
	int vertex[2];
	int ref_count, polygon[2], side[2];
	bool is_round; // for editing
	float weight; // for easify'ing

	// constraints:
	//  Vertex[0] = surf.Polygon[Triangle[0]].Vertex[Side[0]]
	//  Vertex[1] = surf.Polygon[Triangle[0]].Vertex[(Side[0] + 1) % 3]
	//  same for Polygon/Side[1] but Vertex[0 <-> 1]

	float hover_distance(MultiView::Window *win, const vec2 &m, vec3 &tp, float &z) override;
	bool in_rect(MultiView::Window *win, const rect &r) override;
	bool overlap_rect(MultiView::Window *win, const rect &r) override;
};

// only for use in MultiView...
class ModelSkinVertexDummy: public MultiView::SingleData {
};

class ModelBall: public MultiView::SingleData {
public:
	int index;
	float radius;

	float hover_distance(MultiView::Window *win, const vec2 &m, vec3 &tp, float &z) override;
	bool in_rect(MultiView::Window *win, const rect &r) override;
	bool overlap_rect(MultiView::Window *win, const rect &r) override;
};

class ModelCylinder: public MultiView::SingleData {
public:
	int index[2];
	float radius;
	bool round;

	float hover_distance(MultiView::Window *win, const vec2 &m, vec3 &tp, float &z) override;
	bool in_rect(MultiView::Window *win, const rect &r) override;
	bool overlap_rect(MultiView::Window *win, const rect &r) override;
};






class ModelMesh {
public:
	ModelMesh(DataModel *model);
	~ModelMesh();

	DataModel *model;

	void clear();

	// low level (un-action'ed)
	void add_vertex(const vec3 &pos, const ivec4 &bone, const vec4 &weight, int normal_mode, int index = -1);
	void remove_lonely_vertex(int v);
	void _shift_vertex_links(int offset, int delta);
	void _add_polygon(const Array<int> &v, int material, const Array<vec3> &sv, int index = -1);
	void _remove_polygon(int index);

	bool test_sanity(const string &loc);

	void build_topology();

	int add_edge_for_new_polygon(int a, int b, int tria, int side);
	void remove_obsolete_edge(int index);
	void merge_edges();

	MeshInsideTestData *inside_data;
	bool is_inside(const vec3 &p);
	void begin_inside_tests();
	bool inside_test(const vec3 &p);
	void end_inside_tests();
	Array<int> get_boundary_loop(int v0);
	Array<Array<int> > get_connected_components();


	Array<ModelVertex> vertex;
	Array<ModelPolygon> polygon;
	Array<ModelEdge> edge;
	Array<ModelSkinVertexDummy> skin_vertex; // only temporary...

	// purely physical
	Array<ModelBall> ball;
	Array<ModelCylinder> cylinder;



	void on_post_action_update();
	void import_from_triangle_skin(int index);
	void export_to_triangle_mesh(ModelTriangleMesh &trias);
	void get_bounding_box(vec3 &min, vec3 &max, bool dont_reset = false);
	void set_normals_dirty_by_vertices(const Array<int> &index);
	void set_all_normals_dirty();
	void update_normals();
	void clear_selection();
	void selection_from_polygons();
	void selection_from_edges();
	void selection_from_vertices();

	ModelSelection get_selection() const;
	void set_selection(const ModelSelection &s);

	void set_show_vertices(Array<ModelVertex> &vert);
	Array<ModelVertex> show_vertices;


	Geometry copy_geometry();
};

#endif /* SRC_DATA_MODEL_MODELMESH_H_ */
