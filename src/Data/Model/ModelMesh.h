/*
 * ModelMesh.h
 *
 *  Created on: 13.02.2020
 *      Author: michi
 */

#ifndef SRC_DATA_MODEL_MODELMESH_H_
#define SRC_DATA_MODEL_MODELMESH_H_

#include "../../MultiView/SingleData.h"

class DataModel;
class ModelSkin;
class ModelPolygon;
class ModelSelection;
class MeshInsideTestData;


class ModelVertex: public MultiView::SingleData {
public:
	int normal_mode;
	int bone_index;

	bool normal_dirty;
	int ref_count; // polygons

	ModelVertex();
	ModelVertex(const vector &pos);
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

	virtual bool hover(MultiView::Window *win, vector &m, vector &tp, float &z, void *user_data);
	virtual bool inRect(MultiView::Window *win, rect &r, void *user_data);
};

// only for use in MultiView...
class ModelSkinVertexDummy: public MultiView::SingleData {
};

class ModelBall: public MultiView::SingleData {
public:
	int index;
	float radius;
};

class ModelCylinder: public MultiView::SingleData {
public:
	int index[2];
	float radius;
	bool round;
};



#define MODEL_MAX_POLY_FACES			32
#define MODEL_MAX_POLY_EDGES			(MODEL_MAX_POLY_FACES*4)
#define MODEL_MAX_POLY_VERTICES_PER_FACE	16

// DEPRECATED....SOON
struct ModelPolyhedronFace {
	int NumVertices;
	int Index[MODEL_MAX_POLY_VERTICES_PER_FACE];
	plane Plane;
};


// TODO: dynamical!
class ModelPolyhedron: public MultiView::SingleData {
public:
	int NumFaces;
	ModelPolyhedronFace Face[MODEL_MAX_POLY_FACES];
	int NumSVertices;
	int SIndex[MODEL_MAX_POLY_FACES * MODEL_MAX_POLY_VERTICES_PER_FACE];

	// non redundant edge list!
	int NumEdges;
	int EdgeIndex[MODEL_MAX_POLY_EDGES * 2];

	// "topology"
	bool EdgeOnFace[MODEL_MAX_POLY_EDGES * MODEL_MAX_POLY_FACES]; // [edge * NumFaces + face]
	int FacesJoiningEdge[MODEL_MAX_POLY_FACES * MODEL_MAX_POLY_FACES]; // [face1 * NumFaces + face2]
};




class ModelMesh {
public:
	ModelMesh(DataModel *model);
	~ModelMesh();

	DataModel *model;

	void clear();

	// low level (un-action'ed)
	void add_vertex(const vector &pos, int bone, int normal_mode, int index = -1);
	void remove_lonely_vertex(int v);
	void _shift_vertex_links(int offset, int delta);
	void _addPolygon(const Array<int> &v, int material, const Array<vector> &sv, int index = -1);
	void _removePolygon(int index);

	bool test_sanity(const string &loc);

	void build_topology();

	int add_edge_for_new_polygon(int a, int b, int tria, int side);
	void remove_obsolete_edge(int index);
	void merge_edges();

	MeshInsideTestData *inside_data;
	bool is_inside(const vector &p);
	void begin_inside_tests();
	bool inside_test(const vector &p);
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
	Array<ModelPolyhedron> polyhedron; // MERGE WITH POLYGON...



	void on_post_action_update();
	void show_vertices(Array<ModelVertex> &vert);
	void import_from_triangle_skin(int index);
	void exportTo_triangle_skin(ModelSkin &skin);
	void get_bounding_box(vector &min, vector &max, bool dont_reset = false);
	void set_normals_dirty_by_vertices(const Array<int> &index);
	void set_all_normals_dirty();
	void update_normals();
	void clear_selection();
	void selection_from_polygons();
	void selection_from_edges();
	void selection_from_vertices();

	ModelSelection get_selection() const;
	void set_selection(const ModelSelection &s);

};

#endif /* SRC_DATA_MODEL_MODELMESH_H_ */
