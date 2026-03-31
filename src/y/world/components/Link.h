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

namespace ecs {
	struct Entity;
}
struct RigidBody;



enum class LinkType {
	NONE,
	SOCKET,
	HINGE,
	UNIVERSAL,
	SPRING,
	SLIDER
};

struct Link : ecs::Component {
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


