/*
 * ModelPolygon.h
 *
 *  Created on: 28.09.2012
 *      Author: michi
 */

#ifndef MODELPOLYGON_H_
#define MODELPOLYGON_H_

class DataModel;
class ModelVertex;
namespace nix {
	class VertexBuffer;
};

#include "../../x/material.h"
#include "../../MultiView/SingleData.h"

class VertexStagingBuffer {
public:
	Array<vector> p, n;
	Array<float> uv[MATERIAL_MAX_TEXTURES];
	void build(nix::VertexBuffer *vb, int num_textures);
};

class ModelPolygonSide {
public:
	int vertex;
	int edge;
	int edge_direction; // which no of triangle in edge's list are we?
	vector skin_vertex[MATERIAL_MAX_TEXTURES];
	int normal_index;
	vector normal;
	unsigned char triangulation[3];
};

class ModelPolygon: public MultiView::SingleData {
public:
	Array<ModelPolygonSide> side;
	vector temp_normal;
	bool normal_dirty, triangulation_dirty;
	int material;

	float hover_distance(MultiView::Window *win, const vector &m, vector &tp, float &z) override;
	bool in_rect(MultiView::Window *win, const rect &r) override;
	bool overlap_rect(MultiView::Window *win, const rect &r) override;

	Array<int> triangulate(const Array<ModelVertex> &vertex) const;
	void update_triangulation(const Array<ModelVertex> &vertex);
	vector get_normal(const Array<ModelVertex> &vertex) const;
	vector get_area_vector(const Array<ModelVertex> &vertex) const;
	Array<int> get_vertices() const;
	Array<vector> get_skin_vertices() const;
	void invert();
	void add_to_vertex_buffer(const Array<ModelVertex> &vertex, VertexStagingBuffer &vbs, int num_textures);
};


#endif /* MODELPOLYGON_H_ */
