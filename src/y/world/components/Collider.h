/*
 * Collider.h
 *
 *  Created on: Jul 14, 2021
 *      Author: michi
 */

#pragma once

#include "../../y/Component.h"
#include <lib/math/plane.h>
#include <lib/math/vec3.h>
#include <lib/base/pointer.h>

class Model;
class Terrain;

class btCollisionShape;



#define MODEL_MAX_POLY_FACES			32
#define MODEL_MAX_POLY_EDGES			(MODEL_MAX_POLY_FACES*4)
#define MODEL_MAX_POLY_VERTICES_PER_FACE	16


// the face of a polyhedron (=> a polygon)
struct ConvexPolyhedronFace {
	int num_vertices;
	int index[MODEL_MAX_POLY_VERTICES_PER_FACE];
	plane pl; // in model space
};

// a convex polyhedron (for the physical skin)
struct ConvexPolyhedron {
	int num_faces;
	ConvexPolyhedronFace face[MODEL_MAX_POLY_FACES];

	// non redundant vertex list!
	Array<int> vertex;

	// non redundant edge list!
	int num_edges;
	Array<int> edge_index;

	// "topology"
	Array<bool> edge_on_face; // [edge * num_faces + face]
	Array<int> faces_joining_edge; // [face1 * num_faces + face2]
};

// a ball (for the physical skin)
struct Ball {
	int index;
	float radius;
};

// a cylinder (for the physical skin)
struct Cylinder {
	int index[2];
	float radius;
	bool round;
};

// data for collision detection
struct PhysicalMesh : Sharable<base::Empty> {
	Array<int> bone_nr;
	Array<vec3> vertex; // original vertices
	Array<vec3> vertex_dyn; // here the animated vertices are stored before collision detection

	/*int num_triangles;
	unsigned short *triangle_index;*/

	/*int NumEdges;
	unsigned short *EdgeIndex;*/

	Array<Ball> balls;

	Array<Cylinder> cylinders;

	Array<ConvexPolyhedron> poly;
};

// physical skin, but in world coordinates
struct PhysicalMeshAbsolute {
	bool is_ok = true;
	Array<vec3> p;
	Array<plane> pl;
};




class Collider : public Component {
public:
	Collider();
	~Collider() override;

	btCollisionShape* col_shape;

	static const kaba::Class *_class;
};

// requires an already attached Model component
class MeshCollider : public Collider {
public:
	MeshCollider();
	void on_init() override;


	// physical skin (shared)
	shared<PhysicalMesh> phys;
	PhysicalMeshAbsolute phys_absolute;

	static const kaba::Class *_class;
};

class SphereCollider : public Collider {
public:
	SphereCollider();
	static const kaba::Class *_class;
};

class BoxCollider : public Collider {
public:
	BoxCollider();
	static const kaba::Class *_class;
};

// requires an already attached Terrain component
class TerrainCollider : public Collider {
public:
	TerrainCollider();
	void on_init() override;
	Array<float> hh;
	static const kaba::Class *_class;
};

