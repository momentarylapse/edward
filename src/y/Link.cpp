/*
 * Link.cpp
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#include "Link.h"
#include "Object.h"
#include "../lib/file/msg.h"


#if HAS_LIB_BULLET
#include <btBulletDynamicsCommon.h>

btVector3 bt_set_v(const vector &v);
btQuaternion bt_set_q(const quaternion &q);
#endif



Link::Link(LinkType t, Object *_a, Object *_b) : Entity(Type::LINK) {
	link_type = t;
	a = _a;
	b = _b;
	con = nullptr;
}


void Link::_create_link_data(vector &pa, vector &pb, quaternion &iqa, quaternion &iqb, const vector &pos) {
	iqa = a->ang.bar();
	iqb = quaternion::ID;
	pa = iqa * (pos - a->pos);
	pb = pos;
	if (b) {
		iqb = b->ang.bar();
		pb = iqb * (pos - b->pos);
	}
}


LinkSocket::LinkSocket(Object *_a, Object *_b, const vector &pos) : Link(LinkType::SOCKET, _a, _b) {
	vector pa, pb;
	quaternion iqa, iqb;
	_create_link_data(pa, pb, iqa, iqb, pos);
#if HAS_LIB_BULLET
	if (b) {
		//msg_write("-----------add socket 2");
		con = new btPoint2PointConstraint(
			*a->body,
			*b->body,
			bt_set_v(pa),
			bt_set_v(pb));
	} else {
		//msg_write("-----------add socket 1");
		con = new btPoint2PointConstraint(
			*a->body,
			bt_set_v(pa));
	}
#endif
}

LinkHinge::LinkHinge(Object *_a, Object *_b, const vector &pos, const quaternion &ang) : Link(LinkType::HINGE, _a, _b) {
	vector pa, pb;
	quaternion iqa, iqb;
	_create_link_data(pa, pb, iqa, iqb, pos);
#if HAS_LIB_BULLET
	if (b) {
		//msg_write("-----------add hinge 2");
		con = new btHingeConstraint(
			*a->body,
			*b->body,
			bt_set_v(pa),
			bt_set_v(pb),
			bt_set_v(iqa * ang * vector::EZ),
			bt_set_v(iqb * ang * vector::EZ),
			true);
	} else {
		//msg_write("-----------add hinge 1");
		con = new btHingeConstraint(
			*a->body,
			bt_set_v(pa),
			bt_set_v(iqa * ang * vector::EZ),
			true);
	}
#endif
}

LinkUniversal::LinkUniversal(Object *_a, Object *_b, const vector &pos, const quaternion &ang) : Link(LinkType::UNIVERSAL, _a, _b) {
	vector pa, pb;
	quaternion iqa, iqb;
	_create_link_data(pa, pb, iqa, iqb, pos);
	//msg_write("-----------add universal");
#if HAS_LIB_BULLET
	con = new btUniversalConstraint(
		*a->body,
		*b->body,
		bt_set_v(pos),
		bt_set_v(ang * vector::EZ),
		bt_set_v(ang * vector::EY));
	((btUniversalConstraint*)con)->setLimit(4, 0,0.1f);
#endif
}

Link *Link::create(LinkType type, Object *a, Object *b, const vector &pos, const quaternion &ang) {
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
}

Link::~Link() {
}

void Link::set_motor(float v, float max) {
#if HAS_LIB_BULLET
	if (link_type == LinkType::HINGE)
		((btHingeConstraint*)con)->enableAngularMotor(max > 0, v, max);
#endif
}

/*void Link::set_axis(const vector &v) {
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


