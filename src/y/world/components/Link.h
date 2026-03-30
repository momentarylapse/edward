/*
 * Link.h
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#pragma once

#include <ecs/Component.h>

struct vec3;
struct quaternion;

class btTypedConstraint;

class Entity;
class RigidBody;



enum class LinkType {
	NONE,
	SOCKET,
	HINGE,
	UNIVERSAL,
	SPRING,
	SLIDER
};

class Link : public Component {
public:
	//Link();
	~Link() override;

	void set_motor(float v, float max);
	void set_frame(int n, const quaternion &q);

	btTypedConstraint *con = nullptr;
	LinkType link_type = LinkType::NONE;
	int a = -1;
	int b = -1;

	void _create_link_data(vec3 &pa, vec3 &pb, quaternion &iqa, quaternion &iqb, const vec3 &pos);

	//static Link* create(LinkType type, Entity *a, Entity *b, const vec3 &pos, const quaternion &ang);

	void create();
	void create_socket();
	void create_hinge();
	void create_universal();

	static const kaba::Class* _class;
};

/*class LinkSocket : public Link {
public:
	LinkSocket(Entity *a, Entity *b, const vec3 &pos);
	void __init__(Entity *a, Entity *b, const vec3 &pos);
};

class LinkHinge : public Link {
public:
	LinkHinge(Entity *a, Entity *b, const vec3 &pos, const quaternion &ang);
	void __init__(Entity *a, Entity *b, const vec3 &pos, const quaternion &ang);
};

class LinkUniversal : public Link {
public:
	LinkUniversal(Entity *a, Entity *b, const vec3 &pos, const quaternion &ang);
	void __init__(Entity *a, Entity *b, const vec3 &pos, const quaternion &ang);
};*/


