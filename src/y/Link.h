/*
 * Link.h
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#pragma once

#include "../y/BaseClass.h"

class vector;
class quaternion;

class btTypedConstraint;

class Entity;
class SolidBody;



enum class LinkType {
	SOCKET,
	HINGE,
	UNIVERSAL,
	SPRING,
	SLIDER
};

class Link : public BaseClass {
public:
	Link(LinkType type, Entity *a, Entity *b);
	~Link();

	void set_motor(float v, float max);
	void set_frame(int n, const quaternion &q);

	btTypedConstraint *con;
	LinkType link_type;
	SolidBody *a;
	SolidBody *b;

	void _create_link_data(vector &pa, vector &pb, quaternion &iqa, quaternion &iqb, const vector &pos);

	static Link* create(LinkType type, Entity *a, Entity *b, const vector &pos, const quaternion &ang);
};

class LinkSocket : public Link {
public:
	LinkSocket(Entity *a, Entity *b, const vector &pos);
	void __init__(Entity *a, Entity *b, const vector &pos);
};

class LinkHinge : public Link {
public:
	LinkHinge(Entity *a, Entity *b, const vector &pos, const quaternion &ang);
	void __init__(Entity *a, Entity *b, const vector &pos, const quaternion &ang);
};

class LinkUniversal : public Link {
public:
	LinkUniversal(Entity *a, Entity *b, const vector &pos, const quaternion &ang);
	void __init__(Entity *a, Entity *b, const vector &pos, const quaternion &ang);
};


