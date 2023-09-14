/*
 * Layer.h
 *
 *  Created on: Aug 10, 2020
 *      Author: michi
 */

#pragma once

#include "../lib/base/base.h"
#include "../lib/base/pointer.h"
#include "../lib/math/rect.h"
#include "../lib/math/vec2.h"
#include "../lib/image/color.h"

namespace gui {

class Node : public Sharable<VirtualBase> {
public:
	Node(const rect &r);
	virtual ~Node();

	void __init_base__(const rect &r);
	virtual void __delete__();


	enum class Type {
		NODE,
		HBOX,
		VBOX,
		PICTURE,
		TEXT,
		MODEL
	};

	enum Align {
		NONE,
		FILL_X = 1<<0,
		FILL_Y = 1<<1,
		TOP = 1<<2,
		BOTTOM = 1<<3,
		LEFT = 1<<4,
		RIGHT = 1<<5,
		NONSQUARE = 1<<7,
		CENTER_H = 1<<8,
		CENTER_V = 1<<9,
		_FILL_XY = FILL_X | FILL_Y,
		_TOP_LEFT = TOP | LEFT,
	};

	Type type;
	bool visible;
	vec2 pos; // coord system depends on align!
	float width, height;
	rect margin;
	//rect padding;
	Align align;
	color col;
	//float group_alpha;
	float dz;

	color eff_col;
	rect eff_area; // [0:1]x[0:1] system!
	float eff_z;
	bool eff_visible;

	Node *parent;
	shared_array<Node> children;
	void add(shared<Node> );
	void update_geometry(const rect &target);
	void set_area(const rect &r);

	virtual void on_iterate(float dt) {}
	virtual bool on_left_button_down() { return false; }
	virtual bool on_left_button_up() { return false; }
	virtual bool on_middle_button_down() { return false; }
	virtual bool on_middle_button_up() { return false; }
	virtual bool on_right_button_down() { return false; }
	virtual bool on_right_button_up() { return false; }

	virtual void on_enter() {}
	virtual void on_leave() {}
};

class HBox : public Node {
public:
	HBox();
	void __init__();
};

class VBox : public Node {
public:
	VBox();
	void __init__();
};

/*class Spacer : public Node {
public:
	Spacer(float w, float h);
	void __init__(float w, float h);
};


class Layer : public Node {
public:
	Layer(const rect &r);
	void __init__(const rect &r);
};*/

}
