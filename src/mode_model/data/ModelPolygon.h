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

#include <y/world/Material.h>
#include <y/graphics-fwd.h>
#include <multiview/SingleData.h>

class VertexStagingBuffer {
public:
	Array<vec3> p, n;
	Array<float> uv[MATERIAL_MAX_TEXTURES];
	void build(VertexBuffer *vb, int num_textures);
};

struct ModelPolygonSide {
	int vertex;
	int edge;
	int edge_direction; // which no of triangle in edge's list are we?
	vec3 skin_vertex[MATERIAL_MAX_TEXTURES];
	int normal_index;
	vec3 normal;
	unsigned char triangulation[3];
};

struct ModelPolygon: multiview::SingleData {
	Array<ModelPolygonSide> side;
	vec3 temp_normal;
	bool normal_dirty = true;
	bool triangulation_dirty;
	int material = 0;
	int smooth_group = -1;

	// -> MeshSelectionModePolygin.cpp
	/*float hover_distance(MultiView::Window *win, const vec2 &m, vec3 &tp, float &z) override;
	bool in_rect(MultiView::Window *win, const rect &r) override;
	bool overlap_rect(MultiView::Window *win, const rect &r) override;*/

	Array<int> triangulate(const Array<ModelVertex> &vertex) const;
	void update_triangulation(const Array<ModelVertex> &vertex);
	vec3 get_normal(const Array<ModelVertex> &vertex) const;
	vec3 get_area_vector(const Array<ModelVertex> &vertex) const;
	Array<int> get_vertices() const;
	Array<vec3> get_skin_vertices() const;
	void invert();
	void add_to_vertex_buffer(const Array<ModelVertex> &vertex, VertexStagingBuffer &vbs, int num_textures);
};


#endif /* MODELPOLYGON_H_ */
