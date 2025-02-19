//
// Created by Michael Ankele on 2025-02-19.
//

#ifndef POLYGON_H
#define POLYGON_H

#include <multiview/SingleData.h>
#include <lib/base/base.h>
#include <lib/math/vec3.h>
#include <lib/math/vec4.h>
#include <y/world/Material.h>

class MultiViewWindow;
class VertexStagingBuffer;
class vec3;
class mat4;
struct MeshVertex;

struct PolygonSide {
	int vertex;
	vec3 skin_vertex[MATERIAL_MAX_TEXTURES];
	int normal_index;
	int smoothing_id = -1;
	vec3 normal;
	unsigned char triangulation[3];
};

struct Polygon: multiview::SingleData {
	Array<PolygonSide> side;
	vec3 temp_normal;
	bool normal_dirty = true;
	bool triangulation_dirty = true;
	int material = 0;
	int smooth_group = -1;

	// -> MeshSelectionModePolygin.cpp
	/*float hover_distance(MultiView::Window *win, const vec2 &m, vec3 &tp, float &z) override;
	bool in_rect(MultiView::Window *win, const rect &r) override;
	bool overlap_rect(MultiView::Window *win, const rect &r) override;*/

	Array<int> triangulate(const Array<MeshVertex> &vertex) const;
	void update_triangulation(const Array<MeshVertex> &vertex);
	vec3 get_normal(const Array<MeshVertex> &vertex) const;
	vec3 get_area_vector(const Array<MeshVertex> &vertex) const;
	Array<int> get_vertices() const;
	Array<vec3> get_skin_vertices() const;
	void invert();
	void add_to_vertex_buffer(const Array<MeshVertex> &vertex, VertexStagingBuffer &vbs, int num_textures);
};


#endif //POLYGON_H
