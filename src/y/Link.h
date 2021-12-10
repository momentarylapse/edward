/*
 * Link.h
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#pragma once

#include "../y/Entity.h"

class vector;
class quaternion;

class btTypedConstraint;

class Entity3D;
class SolidBody;



enum class LinkType {
	SOCKET,
	HINGE,
	UNIVERSAL,
	SPRING,
	SLIDER
};

class Link : public Entity {
public:
	Link(LinkType type, Entity3D *a, Entity3D *b);
	~Link();

	void set_motor(float v, float max);
	void set_frame(int n, const quaternion &q);

	btTypedConstraint *con;
	LinkType link_type;
	SolidBody *a;
	SolidBody *b;

	void _create_link_data(vector &pa, vector &pb, quaternion &iqa, quaternion &iqb, const vector &pos);

	static Link* create(LinkType type, Entity3D *a, Entity3D *b, const vector &pos, const quaternion &ang);
};

class LinkSocket : public Link {
public:
	LinkSocket(Entity3D *a, Entity3D *b, const vector &pos);
	void __init__(Entity3D *a, Entity3D *b, const vector &pos);
};

class LinkHinge : public Link {
public:
	LinkHinge(Entity3D *a, Entity3D *b, const vector &pos, const quaternion &ang);
	void __init__(Entity3D *a, Entity3D *b, const vector &pos, const quaternion &ang);
};

class LinkUniversal : public Link {
public:
	LinkUniversal(Entity3D *a, Entity3D *b, const vector &pos, const quaternion &ang);
	void __init__(Entity3D *a, Entity3D *b, const vector &pos, const quaternion &ang);
};


