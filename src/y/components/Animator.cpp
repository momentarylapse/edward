/*
 * Animator.cpp
 *
 *  Created on: Jul 14, 2021
 *      Author: michi
 */

#include "Animator.h"
#include "Skeleton.h"
#include "../Model.h"
#include "../ModelManager.h"
#include "../../y/Entity.h"
#include "../../lib/os/msg.h"
#include "../../graphics-impl.h"


const kaba::Class *Animator::_class = nullptr;



MetaMove::MetaMove() {
	num_frames_skeleton = 0;
	num_frames_vertex = 0;
}


Animator::Animator() {
	buf = nullptr;

	// "auto-animate"
	auto_animated = true;
	num_operations = 0;
	operation[0].move = 0;
	operation[0].time = 0;
	operation[0].command = MoveOperation::Command::SET;
	operation[0].param1 = 0;
	operation[0].param2 = 0;
	/*if (anim.meta) {
		for (int i=0; i<anim.meta->move.num; i++)
			if (anim.meta->move[i].num_frames > 0) {
				anim.operation[0].move = i;
				break;
			}
	}*/

}

Animator::~Animator() {
	if (buf)
		delete buf;
}

void Animator::on_init() {
	auto m = owner->get_component<Model>();

	meta = m->_template->animator->meta;

	// skeleton
	auto sk = m->owner->get_component<Skeleton>();
	dmatrix.resize(sk->bones.num);
	for (int i=0; i<sk->bones.num; i++) {
		dmatrix[i] = mat4::translation(sk->pos0[i]);
	}

#ifdef USING_OPENGL
	buf = new UniformBuffer;
#endif
#ifdef USING_VULKAN
	buf = new UniformBuffer(sk->bones.num * sizeof(mat4));
#endif
}




void Animator::do_animation(float elapsed) {
	[[maybe_unused]] auto m = owner->get_component<Model>();
	auto sk = owner->get_component<Skeleton>();

	// recursion
	//for (auto &b: sk->bone)
	//	if (auto bm = b.get_component<Model>()) {
			//bm->_matrix = b.get_matrix();

			// done by global Animator[] list iteration
			//b.model->do_animation(elapsed);
	//	}

	if (!meta)
		return;


	int num_ops = num_operations;
	if (auto_animated) {
		// default: just run a single animation
		_add_time(0, elapsed, 0, true);
		num_ops = 1;
	}

// vertex animation

	[[maybe_unused]] bool vertex_animated = false;
	for (int op=0;op<num_ops;op++){
		if (operation[op].move < 0)
			continue;
		Move *m = &meta->move[operation[op].move];
		//msg_write(GetFilename() + format(" %d %d %p %d", op, anim.operation[op].move, m, m->num_frames));
		if (m->num_frames == 0)
			continue;


		if (m->type == AnimationType::VERTEX){
			vertex_animated = true;

			// frame data
			[[maybe_unused]] int fr = (int)(operation[op].time); // current frame (relative)
			[[maybe_unused]] float dt = operation[op].time - (float)fr;
			[[maybe_unused]] int f1 = m->frame0 + fr; // current frame (absolute)
			[[maybe_unused]] int f2 = m->frame0 + (fr+1)%m->num_frames; // next frame (absolute)

		}
	}

// skeletal animation

	for (int i=0;i<sk->bones.num;i++){
		auto *b = &sk->bones[i];

		// operations
		for (int iop=0;iop<num_ops;iop++){
			MoveOperation *op = &operation[iop];
			if (op->move < 0)
				continue;
			Move *move = &meta->move[op->move];
			if (move->num_frames == 0)
				continue;
			if (move->type != AnimationType::SKELETAL)
				continue;
			quaternion w,w0,w1,w2,w3;
			vec3 p,p1,p2;

		// calculate the alignment belonging to this argument
			float t = max(op->time, 0.0f);
			int fr = (int)t; // current frame (relative)
			int f1 = move->frame0 + fr; // current frame (absolute)
			int f2 = move->frame0 + (fr+1)%move->num_frames; // next frame (absolute)
			float df = t-(float)fr; // time since start of current frame
			//msg_write(format("%d   %d  %d    %d", i, f1, sk->bones.num, meta->skel_ang.num));
			w1 = meta->skel_ang[f1 * sk->bones.num + i]; // first value
			p1 = meta->skel_dpos[f1 * sk->bones.num + i];
			w2 = meta->skel_ang[f2 * sk->bones.num + i]; // second value
			p2 = meta->skel_dpos[f2 * sk->bones.num + i];
			move->inter_quad = false;
			/*if (m->InterQuad){
				w0=m->ang[i][(f-1+m->NumFrames)%m->NumFrames]; // last value
				w3=m->ang[i][(f+2             )%m->NumFrames]; // third value
				// interpolate the current alignment
				w = quaternion::interpolate(w0,w1,w2,w3,df);
				p=(1.0f-df)*p1+df*p2 + SkeletonDPos[i];
			}else*/{
				// interpolate the current alignment
				w = quaternion::interpolate(w1,w2,df);
				p = (1.0f-df)*p1 + df*p2 + sk->dpos[i];
			}


		// execute the operations

			// overwrite
			if (op->command == op->Command::SET){
				b->ang = w;
				b->pos = p;

			// overwrite, if current doesn't equal 0
			}else if (op->command == op->Command::SET_NEW_KEYED){
				if (w.w != 1)
					b->ang = w;
				if (p != v_0)
					b->pos = p;

			// overwrite, if last equals 0
			}else if (op->command == op->Command::SET_OLD_KEYED){
				if (b->ang.w==1)
					b->ang=w;
				if (b->pos==v_0)
					b->pos=p;

			// w = w_old         + w_new * f
			}else if (op->command == op->Command::ADD_1_FACTOR){
				w = w.scale_angle(op->param1);
				b->ang = w * b->ang;
				b->pos += op->param1 * p;

			// w = w_old * (1-f) + w_new * f
			}else if (op->command == op->Command::MIX_1_FACTOR){
				b->ang = quaternion::interpolate(b->ang, w, op->param1);
				b->pos = (1 - op->param1) * b->pos + op->param1 * p;

			// w = w_old * a     + w_new * b
			}else if (op->command == op->Command::MIX_2_FACTOR){
				b->ang = b->ang.scale_angle(op->param1);
				w = w.scale_angle(op->param2);
				b->ang = quaternion::interpolate(b->ang, w, 0.5f);
				b->pos = op->param1 * b->pos + op->param2 * p;
			}
		}

		// bone has parent -> align to parent
		//vec3 dpos = b->delta_pos;
		auto t0 = mat4::translation(- sk->pos0[i]);
		if (sk->parents[i] >= 0) {
			b->pos = sk->bones[sk->parents[i]].pos + sk->bones[sk->parents[i]].ang * sk->dpos[i];
			//dpos = b->cur_pos - b->rest_pos;
		}

		// create matrices (model -> skeleton)
		auto t = mat4::translation(b->pos);
		auto r = mat4::rotation(b->ang);
		//b->dmatrix = t * r;
		dmatrix[i] = t * r * t0;
	}
}



// reset all animation data for a model (needed in each frame before applying animations!)
void Animator::reset() {
	num_operations = 0;
	auto_animated = false;
}

// did the animation reach its end?
bool Animator::is_done(int operation_no) {
	int move_no = operation[operation_no].move;
	if (move_no < 0)
		return true;
	// in case animation doesn't exist
	if (meta->move[move_no].num_frames == 0)
		return true;
	return (operation[operation_no].time >= (float)(meta->move[move_no].num_frames - 1));
}


// dumbly add the correct animation time, ignore animation's ending
void Animator::_add_time(int operation_no, float elapsed, float v, bool loop) {
	auto &op = operation[operation_no];
	int move_no = op.move;
	if (move_no < 0)
		return;
	Move *move = &meta->move[move_no];
	if (move->num_frames == 0)
		return; // in case animation doesn't exist

	// how many frames have passed
	float dt = elapsed * ( move->frames_per_sec_const + move->frames_per_sec_factor * v );

	// add the correct time
	op.time += dt;
	// time may now be way out of range of the animation!!!

	if (is_done(operation_no)) {
		if (loop)
			op.time -= float(move->num_frames) * (int)((float)(op.time / move->num_frames));
		else
			op.time = (float)(move->num_frames) - 1;
	}
}

// apply an animate to a model
//   a new animation "layer" is being added for mixing several animations
//   the variable <time> is being increased
bool Animator::add_x(MoveOperation::Command cmd, float param1, float param2, int move_no, float &time, float dt, float vel_param, bool loop) {
	if (!meta)
		return false;
	if (num_operations >= MODEL_MAX_MOVE_OPS - 1) {
		msg_error("Animator.add(): no more than " + i2s(MODEL_MAX_MOVE_OPS) + " animation layers allowed");
		return false;
	}
	int index = -1;
	foreachi (auto &m, meta->move, i)
		if (m.id == move_no)
			index = i;
	if (index < 0) {
		msg_error("move id not existing: " + i2s(move_no));
		foreachi (auto &m, meta->move, i)
			msg_write(m.id);
		return false;
	}
	int n = num_operations ++;
	operation[n].move = index;
	operation[n].command = cmd;
	operation[n].param1 = param1;
	operation[n].param2 = param2;
	operation[n].time = time;

	_add_time(n, dt, vel_param, loop);
	time = operation[n].time;
	return is_done(n);
}

bool Animator::add(MoveOperation::Command cmd, int move_no, float &time, float dt, bool loop) {
	return add_x(cmd, 0, 0, move_no, time, dt, 0, loop);

}

// get the number of frames for a particular animation
int Animator::get_frames(int move_no) {
	if (!meta)
		return 0;
	return meta->move[move_no].num_frames;
}


vec3 Animator::get_vertex(int index) {
	auto m = owner->get_component<Model>();
	auto sk = owner->get_component<Skeleton>();
	auto s = m->mesh[MESH_HIGH];
	int b = s->bone_index[index].i;
	return m->owner->get_matrix() * dmatrix[b] * s->vertex[index] - sk->pos0[b];
}
