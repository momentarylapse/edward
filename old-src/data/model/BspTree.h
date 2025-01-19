/*
 * BspTree.h
 *
 *  Created on: 23.10.2013
 *      Author: michi
 */

#ifndef BSPTREE_H_
#define BSPTREE_H_

struct BspBranch;
class ModelMesh;
class ModelPolygon;
class vec3;

class BspTree {
public:
	BspTree();
	BspBranch *b;
	~BspTree();
	void add(ModelPolygon &p, ModelMesh *m, float epsilon);
	bool inside(const vec3 &v);
};

#endif /* BSPTREE_H_ */
