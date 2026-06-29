/*
 * Layer.h
 *
 *  Created on: Aug 10, 2020
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <lib/math/rect.h>
#include <lib/math/vec2.h>
#include <lib/image/color.h>
#include <lib/layout/Node.h>

namespace layout {
	struct Resource;
}

namespace gui {

class Node : public layout::Node {
public:
	explicit Node();
	~Node() override;


	enum class Type {
		NODE,
		HBOX,
		VBOX,
		PICTURE,
		TEXT,
		MODEL,
		CANVAS
	};

	Type type;

	color col;
	float dz;

	color eff_col;
	float eff_z;
	bool allow_hover;

	Node *parent;
	shared_array<Node> children;

	void add(shared<Node> n);
	void add_from_source(const string& source);
	void apply_resource(const layout::Resource& r);
	void remove(Node &n);
	void remove_all_children();
	Node* get(const string &id);

	void set_area(const rect& area);

	Array<const layout::Node *> _get_children(layout::ChildFilter f) const override;
	void negotiate_content_area(const rect &available) override;
	void set_option(const string& k, const string& v) override;

	virtual void on_iterate(float dt) {}
	/*virtual bool on_left_button_down() { return false; }
	virtual bool on_left_button_up() { return false; }
	virtual bool on_middle_button_down() { return false; }
	virtual bool on_middle_button_up() { return false; }
	virtual bool on_right_button_down() { return false; }
	virtual bool on_right_button_up() { return false; }*/

	virtual void on_enter(const vec2& m) {}
	virtual void on_leave() {}
};

class HBox : public Node {
public:
	HBox();

	/*vec2 get_greed_factor() const override;
	vec2 get_content_min_size() const override;
	void negotiate_content_area(const rect& available) override;*/
	float spacing = 0;
};

class VBox : public Node {
public:
	VBox();
	vec2 get_greed_factor() const override;
	vec2 get_content_min_size() const override;
	void negotiate_content_area(const rect& available) override;
	void set_option(const string &key, const string &value) override;
	Array<float> get_min_heights() const;
	float spacing = 0;
};

}
