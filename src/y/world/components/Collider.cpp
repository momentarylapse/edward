/*
 * Collider.cpp
 *
 *  Created on: Jul 14, 2021
 *      Author: michi
 */

#include "Collider.h"
#include "../Model.h"
#include "../ModelManager.h"
#include "../Terrain.h"
#include "../../y/Entity.h"
#include <lib/base/set.h>
#include <lib/math/quaternion.h>
#include <lib/os/msg.h>

#if HAS_LIB_BULLET
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>


btVector3 bt_set_v(const vec3 &v);
btTransform bt_set_trafo(const vec3 &p, const quaternion &q);
#endif


const kaba::Class *Collider::_class = nullptr;
const kaba::Class *MeshCollider::_class = nullptr;
const kaba::Class *SphereCollider::_class = nullptr;
const kaba::Class *BoxCollider::_class = nullptr;
const kaba::Class *TerrainCollider::_class = nullptr;

Collider::Collider() {
	col_shape = nullptr;
}

Collider::~Collider() {
#if HAS_LIB_BULLET
	if (col_shape)
		delete col_shape;
#endif
}


MeshCollider::MeshCollider() {
	// physical
	if (phys and !phys_is_reference)
		delete phys;
}

void MeshCollider::on_init() {
	auto m = owner->get_component<Model>();
	if (!m) {
		if (auto r = owner->get_component<ModelRef>())
			m = r->model;
	}
	if (!m) {
		msg_error("MeshCollider without Model");
		return;
	}

	phys = m->_template->mesh_collider->phys;
	phys_is_reference = true;



#if HAS_LIB_BULLET
	if (phys->balls.num + phys->cylinders.num + phys->poly.num > 0) {
		auto comp = new btCompoundShape(false, 0);
		for (auto &b: phys->balls) {
			vec3 a = phys->vertex[b.index];
			auto bb = new btSphereShape(btScalar(b.radius));
			comp->addChildShape(bt_set_trafo(a, quaternion::ID), bb);
		}
		for (auto &c: phys->cylinders) {
			vec3 a = phys->vertex[c.index[0]];
			vec3 b = phys->vertex[c.index[1]];
			auto cc = new btCylinderShapeZ(bt_set_v(vec3(c.radius, c.radius, (b - a).length() / 2)));
			auto q = quaternion::rotation((a-b).dir2ang());
			comp->addChildShape(bt_set_trafo((a+b)/2, q), cc);
			if (c.round) {
				auto bb1 = new btSphereShape(btScalar(c.radius));
				comp->addChildShape(bt_set_trafo(a, quaternion::ID), bb1);
				auto bb2 = new btSphereShape(btScalar(c.radius));
				comp->addChildShape(bt_set_trafo(b, quaternion::ID), bb2);
			}
		}
		for (auto &p: phys->poly) {
			if (true){
				base::set<int> vv;
				for (int i=0; i<p.num_faces; i++)
					for (int k=0; k<p.face[i].num_vertices; k++){
						vv.add(p.face[i].index[k]);
					}
				// btConvexPointCloudShape not working!
				auto pp = new btConvexHullShape();
				for (int i: vv)
					pp->addPoint(bt_set_v(phys->vertex[i]));
				comp->addChildShape(bt_set_trafo(v_0, quaternion::ID), pp);
			} else {
				// ARGH, btConvexPointCloudShape not working
				//   let's use a crude box for now... (-_-)'
				vec3 a, b;
				a = b = phys->vertex[p.face[0].index[0]];
				for (int i=0; i<p.num_faces; i++)
					for (int k=0; k<p.face[i].num_vertices; k++){
						auto vv = phys->vertex[p.face[i].index[k]];
						a._min(vv);
						b._max(vv);
					}
				auto pp = new btBoxShape(bt_set_v((b-a) / 2));
				comp->addChildShape(bt_set_trafo((a+b)/2, quaternion::ID), pp);

			}
		}
		col_shape = comp;
	}

	/*if (phys->balls.num > 0) {
		auto &b = phys->balls[0];
		o->colShape = new btSphereShape(btScalar(b.radius));
	} else if (phys->cylinders.num > 0) {
		auto &c = phys->cylinders[0];
		vec3 a = o->mesh[0]->vertex[c.index[0]];
		vec3 b = o->mesh[0]->vertex[c.index[1]];
		o->colShape = new btCylinderShapeZ(bt_set_v(vec3(c.radius, c.radius, (b - a).length())));
	} else if (phys->poly.num > 0) {

	} else {
	}*/
#endif
}


TerrainCollider::TerrainCollider() = default;

void TerrainCollider::on_init() {
	auto t = owner->get_component<Terrain>();
	if (!t) {
		if (auto r = owner->get_component<TerrainRef>())
			t = r->terrain;
	}
	if (!t) {
		msg_error("TerrainCollider without Terrain!");
		return;
	}

	float a = 0, b = 0;
	for (float f: t->height){
		a = min(a, f);
		b = max(b, f);
	}
	float d = max(abs(a), abs(b)) + 10;

	//tt->colShape = new btStaticPlaneShape(btVector3(0,1,0), 0);

	// transpose the array for bullet
	for (int z=0; z<t->num_z+1; z++)
		for (int x=0; x<t->num_x+1; x++)
			hh.add(t->height[x * (t->num_z+1) + z]);
	// data is only referenced by bullet!  (keep)

#if HAS_LIB_BULLET
	auto hf = new btHeightfieldTerrainShape(t->num_x+1, t->num_z+1, hh.data, 1.0f, -d, d, 1, PHY_FLOAT, false);
	hf->setLocalScaling(bt_set_v(t->pattern + vec3(0,1,0)));

	// bullet assumes the origin in the center of the terrain!
	auto comp = new btCompoundShape(false, 0);
	comp->addChildShape(bt_set_trafo(vec3(t->pattern.x * t->num_x, 0, t->pattern.z * t->num_z)/2, quaternion::ID), hf);
	col_shape = comp;
#endif
}
