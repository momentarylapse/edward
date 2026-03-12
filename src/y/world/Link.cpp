/*
 * Link.cpp
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#include "Link.h"
#include <ecs/Entity.h>
#include <lib/os/msg.h>
#include "components/SolidBody.h"


#if HAS_LIB_BULLET
#include <btBulletDynamicsCommon.h>

btVector3 bt_set_v(const vec3 &v);
btQuaternion bt_set_q(const quaternion &q);
#endif

const kaba::Class* Link::_class = nullptr;


/*Link::Link(LinkType t, Entity *_a, Entity *_b) : BaseClass(Type::LINK) {
	link_type = t;
	a = nullptr;
	b = nullptr;
	if (_a)
		a = _a->get_component<SolidBody>();
	if (_b)
		b = _b->get_component<SolidBody>();
	con = nullptr;
}*/


void Link::_create_link_data(vec3 &pa, vec3 &pb, quaternion &iqa, quaternion &iqb, const vec3 &pos) {
	iqa = a->ang.bar();
	iqb = quaternion::ID;
	pa = iqa * (pos - a->pos);
	pb = pos;
	if (b) {
		iqb = b->ang.bar();
		pb = iqb * (pos - b->pos);
	}
}

void Link::create() {
	switch (link_type) {
		case LinkType::SOCKET:
			create_socket();
			break;
		case LinkType::HINGE:
			create_hinge();
			break;
		case LinkType::UNIVERSAL:
			create_universal();
			break;
		default:
			break;
	}
}


void Link::create_socket() {
	if (!a->get_component<SolidBody>())
		return;
	vec3 pa, pb;
	quaternion iqa, iqb;
	_create_link_data(pa, pb, iqa, iqb, owner->pos);
#if HAS_LIB_BULLET
	if (b) {
		//msg_write("-----------add socket 2");
		con = new btPoint2PointConstraint(
			*a->get_component<SolidBody>()->body,
			*b->get_component<SolidBody>()->body,
			bt_set_v(pa),
			bt_set_v(pb));
	} else {
		//msg_write("-----------add socket 1");
		con = new btPoint2PointConstraint(
			*a->get_component<SolidBody>()->body,
			bt_set_v(pa));
	}
#endif
}

void Link::create_hinge() {
	if (!a->get_component<SolidBody>())
		return;
	vec3 pa, pb;
	quaternion iqa, iqb;
	_create_link_data(pa, pb, iqa, iqb, owner->pos);
#if HAS_LIB_BULLET
	if (b) {
		//msg_write("-----------add hinge 2");
		con = new btHingeConstraint(
			*a->get_component<SolidBody>()->body,
			*b->get_component<SolidBody>()->body,
			bt_set_v(pa),
			bt_set_v(pb),
			bt_set_v(iqa * owner->ang * vec3::EZ),
			bt_set_v(iqb * owner->ang * vec3::EZ),
			true);
	} else {
		//msg_write("-----------add hinge 1");
		con = new btHingeConstraint(
			*a->get_component<SolidBody>()->body,
			bt_set_v(pa),
			bt_set_v(iqa * owner->ang * vec3::EZ),
			true);
	}
#endif
}

void Link::create_universal() {
	if (!a->get_component<SolidBody>() or !b->get_component<SolidBody>())
		return;
	vec3 pa, pb;
	quaternion iqa, iqb;
	_create_link_data(pa, pb, iqa, iqb, owner->pos);
	//msg_write("-----------add universal");
#if HAS_LIB_BULLET
	con = new btUniversalConstraint(
		*a->get_component<SolidBody>()->body,
		*b->get_component<SolidBody>()->body,
		bt_set_v(owner->pos),
		bt_set_v(owner->ang * vec3::EZ),
		bt_set_v(owner->ang * vec3::EY));
	((btUniversalConstraint*)con)->setLimit(4, 0,0.1f);
#endif
}

/*Link *Link::create(LinkType type, Entity *a, Entity *b, const vec3 &pos, const quaternion &ang) {
	msg_write(format("LINK   %d   %s  %s", (int)type, p2s(a), p2s(b)));
	if (type == LinkType::SOCKET) {
		return new LinkSocket(a, b, pos);
	} else if (type == LinkType::HINGE) {
		return new LinkHinge(a, b, pos, ang);
	} else if (type == LinkType::UNIVERSAL) {
		return new LinkUniversal(a, b, pos, ang);
	} else {
		throw Exception("unknown link: " + i2s((int)type));
	}
	return nullptr;
}*/

Link::~Link() = default;

void Link::set_motor(float v, float max) {
#if HAS_LIB_BULLET
	if (link_type == LinkType::HINGE) {
		((btHingeConstraint*)con)->enableAngularMotor(max > 0, v, max);
	}
#endif
}

/*void Link::set_axis(const vec3 &v) {
	auto vv = bt_set_v(v);
	if (type == LinkType::HINGE)
		((btHingeConstraint*)con)->setAxis(vv);
	btTransform f = bt_set_trafo(v_0, quaternion::ID);
	((btHingeConstraint*)con)->setFrames(f,f);
}*/

void Link::set_frame(int n, const quaternion &q) {
#if HAS_LIB_BULLET
	if (link_type == LinkType::HINGE) {
		if (n == 1)
			((btHingeConstraint*)con)->getBFrame().setRotation(bt_set_q(q));
		else
			((btHingeConstraint*)con)->getAFrame().setRotation(bt_set_q(q));
	}
#endif
}


