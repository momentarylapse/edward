/*
 * Animator.h
 *
 *  Created on: Jul 14, 2021
 *      Author: michi
 */
#pragma once

#include "../../y/Component.h"
#include <lib/ygraphics/graphics-fwd.h>
#include <lib/base/base.h>
#include <lib/base/pointer.h>

class Model;
struct vec3;
struct quaternion;
struct mat4;
class MetaMove;



enum class AnimationType {
	NONE,
	VERTEX,
	SKELETAL
};

// single animation
class Move {
public:
	string name;
	int id;
	AnimationType type;
	int num_frames;
	int frame0;

	// properties
	float frames_per_sec_const, frames_per_sec_factor;
	bool inter_quad, inter_loop;
};

// a list of animations
class MetaMove : public Sharable<base::Empty> {
public:
	MetaMove();
	// universal animation data
	Array<Move> move;

	int num_frames_skeleton, num_frames_vertex;


	// skeletal animation data
	//Array<Array<vec3>> skel_dpos; //   [frame,bone]
	//Array<Array<quaternion>> skel_ang; //   [frame,bone]
	Array<vec3> skel_dpos;
	Array<quaternion> skel_ang;

	// vertex animation data
	struct {
		//Array<Array<vec3>> dpos; // vertex animation data   [frame,vertex]
		Array<vec3> dpos;
	} mesh[4];
};



#define MODEL_MAX_MOVE_OPS				8


// commands for animation (move operations)
class MoveOperation {
public:
	// move operations
	enum class Command {
		SET,			// overwrite
		SET_NEW_KEYED,	// overwrite, if current doesn't equal 0
		SET_OLD_KEYED,	// overwrite, if last equals 0
		ADD_1_FACTOR,	// w = w_old         + w_new * f
		MIX_1_FACTOR,	// w = w_old * (1-f) + w_new * f
		MIX_2_FACTOR	// w = w_old * a     + w_new * b
	};
	int move;
	Command command;
	float time, param1, param2;
};

class Animator : public Component {
public:
	Animator();
	~Animator() override;

	void on_init() override;

	// move operations
	int num_operations;
	bool auto_animated;
	MoveOperation operation[MODEL_MAX_MOVE_OPS];
	shared<MetaMove> meta;

	// dynamical data (own)
	//Mesh *mesh[MODEL_NUM_MESHES]; // here the animated vertices are stored before rendering

	Array<mat4> dmatrix;
	ygfx::UniformBuffer* buf;

	// animation
	void _cdecl reset();
	bool _cdecl is_done(int operation_no);
	bool _cdecl add_x(MoveOperation::Command cmd, float param1, float param2, int move_no, float &time, float dt, float vel_param, bool loop);
	bool _cdecl add(MoveOperation::Command cmd, int move_no, float &time, float dt, bool loop);
	int _cdecl get_frames(int move_no);
	void do_animation(float elapsed);

	void _add_time(int operation_no, float elapsed, float v, bool loop);



	vec3 get_vertex(int index);

	static const kaba::Class *_class;
};
