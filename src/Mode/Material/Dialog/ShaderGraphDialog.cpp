/*
 * ShaderGraphDialog.cpp
 *
 *  Created on: 21.02.2020
 *      Author: michi
 */

#include "ShaderGraphDialog.h"
#include "../ModeMaterial.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/ColorScheme.h"
#include "../../../Data/Material/DataMaterial.h"
#include "../../../Data/Material/ShaderGraph.h"
#include "../../../lib/math/rect.h"
#include "../../../lib/nix/nix.h"


const int NODE_WIDTH = 150;
const int NODE_HEADER_HEIGHT = 25;
const int NODE_PORT_HEIGHT = 20;

const Array<string> NODE_TYPES = {"Color", "Texture", "ColorMultiply", "Output"};

ShaderGraphDialog::ShaderGraphDialog(hui::Window *parent, DataMaterial *_data) :
	hui::Dialog("Shader Graph", 1000, 600, parent, true)
{
	data = _data;

	node_moving = nullptr;
	move_dx = move_dy = -1;
	graph = new ShaderGraph();
	//graph->add("Basic", 300, 50);
	//graph->add("Texture", 100, 50);

	auto n1 = graph->add("Texture", 50, 50);
	auto n2 = graph->add("Color", 50, 300);
	auto n3 = graph->add("ColorMultiply", 250, 150);
	auto n4 = graph->add("Output", 450, 50);
	graph->connect(n1, 0, n3, 0);
	graph->connect(n2, 0, n3, 1);
	graph->connect(n3, 0, n4, 0);

	from_source("Grid grid '' vertical\n"\
			"\tDrawingArea area '' grabfocus expandx expandy\n"\
			"\tGrid bb '' buttonbar\n"\
			"\t\tButton update 'Update'");
	event_xp("area", "hui:draw", [=](Painter *p){ on_draw(p); });
	//event_x("area", "hui:mouse-move", [=]{ on_mouse_move(); });
	//event_x("area", "hui:left-button-down", [=]{ on_left_button_down(); });
	//event_x("area", "hui:left-button-up", [=]{ on_left_button_up(); });
	//event_x("area", "hui:key-down", [=]{ on_key_down(); });
	event("update", [=]{ on_update(); });

	popup = new hui::Menu();
	for (int i=0; i<NODE_TYPES.num; i++) {
		popup->add(NODE_TYPES[i], "add-node-" + i2s(i));
		event("add-node-" + i2s(i), [=]{
			graph->add(NODE_TYPES[i], 400, 200);
		});
	}

	on_update();
}

ShaderGraphDialog::~ShaderGraphDialog() {
	delete graph;
}

int node_get_in_y(ShaderNode *n, int i) {
	return n->y + NODE_HEADER_HEIGHT + i * NODE_PORT_HEIGHT + NODE_PORT_HEIGHT/2;
}

int node_get_out_y(ShaderNode *n, int i) {
	return n->y + NODE_HEADER_HEIGHT + n->params.num * NODE_PORT_HEIGHT + i * NODE_PORT_HEIGHT + NODE_PORT_HEIGHT/2;
}

rect node_get_param_rect(ShaderNode *n, int i) {
	int y = node_get_in_y(n, i);
	return rect(n->x + NODE_WIDTH / 2, n->x + NODE_WIDTH-10, y-8, y+8);
}

rect node_get_out_rect(ShaderNode *n, int i) {
	int y = node_get_out_y(n, i);
	return rect(n->x + NODE_WIDTH-10, n->x + NODE_WIDTH+20, y-10, y+10);
}

rect node_get_in_rect(ShaderNode *n, int i) {
	int y = node_get_in_y(n, i);
	return rect(n->x-20, n->x + 10, y-10, y+10);
}

rect node_area(ShaderNode *n) {
	int h = NODE_HEADER_HEIGHT + n->params.num * NODE_PORT_HEIGHT + n->output.num * NODE_PORT_HEIGHT;
	return rect(n->x, n->x + NODE_WIDTH, n->y, n->y + h);
}

void ShaderGraphDialog::draw_node(Painter *p, ShaderNode *n) {
	p->set_color(scheme.GRID);
	p->draw_rect(node_area(n));

	p->set_color(scheme.TEXT);
	if (n == hover.node)
		p->set_color(scheme.hoverify(scheme.TEXT));
	p->set_font_size(15);
	p->draw_str(n->x + NODE_WIDTH / 2 - p->get_str_width(n->type) / 2, n->y+3, n->type);

	p->set_font_size(11);

	// in
	foreachi (auto &pp, n->params, i) {
		p->set_color(scheme.TEXT);
		float y = node_get_in_y(n, i);
		p->draw_str(n->x + 20, y, pp.name);// + ": " + shader_value_type_to_str(pp.type));
		if (pp.type == ShaderValueType::COLOR) {
			p->set_color(pp.get_color());
			p->draw_rect(node_get_param_rect(n, i));
		}
		p->set_color(scheme.TEXT);
		if (graph->find_source(n, i))
			p->set_color(ColorInterpolate(scheme.TEXT, scheme.GRID, 0.5f));
		if (n == hover.node and i == hover.param)
			p->set_color(scheme.hoverify(scheme.TEXT));
		p->draw_str(n->x + NODE_WIDTH / 2, y, pp.value);

		p->set_color(scheme.TEXT);
		if (n == hover.node and i == hover.port_in)
			p->set_color(scheme.hoverify(scheme.TEXT));
		p->draw_circle(n->x - 5, y, 5);
	}

	// out
	foreachi (auto &pp, n->output, i) {
		float y = node_get_out_y(n, i);
		p->set_color(scheme.TEXT);
		p->draw_str(n->x + 40, y, "out: " + pp.name);// + ": " + shader_value_type_to_str(pp.type));
		if (n == hover.node and i == hover.port_out)
			p->set_color(scheme.hoverify(scheme.TEXT));
		p->draw_circle(n->x + NODE_WIDTH + 5, y, 5);
	}
}

void ShaderGraphDialog::on_draw(Painter *p) {
	int w = p->width;
	int h = p->height;
	p->set_color(scheme.BACKGROUND);
	p->draw_rect(p->area());

	for (auto *n: graph->nodes)
		draw_node(p, n);

	p->set_color(Blue);
	for (auto &l: graph->links) {
		p->draw_line(l.source->x + NODE_WIDTH + 5, node_get_out_y(l.source, l.source_port),
				l.dest->x - 5, node_get_in_y(l.dest, l.dest_port));
	}

	if (new_link.node) {
		auto e = hui::GetEvent();
		p->set_color(Red);
		p->draw_str(20,20,format("%d %d   ->  %d %d", new_link.port, new_link.is_source, hover.port_in, hover.port_out));
		if (new_link.is_source)
			p->draw_line(new_link.node->x + NODE_WIDTH + 5, node_get_out_y(new_link.node, new_link.port), e->mx, e->my);
		else
			p->draw_line(new_link.node->x - 5, node_get_in_y(new_link.node, new_link.port), e->mx, e->my);
	}

	p->set_font_size(11);
	p->set_color(scheme.TEXT);
	p->draw_str(600, 10, graph->build_fragment_source());
}

void ShaderGraphDialog::on_key_down() {
	auto e = hui::GetEvent();
	if (e->key_code == hui::KEY_DELETE)
		if (node_moving) {
			graph->remove(node_moving);
			node_moving = nullptr;
		}
	redraw("area");
}

void ShaderGraphDialog::on_left_button_down() {
	auto e = hui::GetEvent();
	hover = get_hover();
	if (hover.node) {
		if (hover.port_in >= 0) {
			graph->unconnect(nullptr, -1, hover.node, hover.port_in);
			new_link.node = hover.node;
			new_link.port = hover.port_in;
			new_link.is_source = false;
		} else if (hover.port_out >= 0) {
			graph->unconnect(hover.node, hover.port_out, nullptr, -1);
			new_link.node = hover.node;
			new_link.port = hover.port_out;
			new_link.is_source = true;
		} else if (hover.param >= 0) {
			auto &pp = hover.node->params[hover.param];
			if (pp.type == ShaderValueType::COLOR) {
				color col = pp.get_color();
				if (hui::SelectColor(this, col)) {
					pp.set_color(hui::Color);
				}
			}
		} else {
			node_moving = hover.node;
			move_dx = e->mx - hover.node->x;
			move_dy = e->my - hover.node->y;
		}
	}
	redraw("area");
}

void ShaderGraphDialog::on_left_button_up() {
	if (new_link.node) {
		if (new_link.is_source and hover.node and hover.port_in >= 0) {
			graph->connect(new_link.node, new_link.port, hover.node, hover.port_in);
		} else if (!new_link.is_source and hover.node and hover.port_out >= 0) {
			graph->connect(hover.node, hover.port_out, new_link.node, new_link.port);
		}
	}
	node_moving = nullptr;
	new_link = NewLinkData();
	redraw("area");
}

void ShaderGraphDialog::on_mouse_move() {
	auto e = hui::GetEvent();
	hover = get_hover();
	if (node_moving) {
		node_moving->x = e->mx - move_dx;
		node_moving->y = e->my - move_dy;
	}
	redraw("area");
}

void ShaderGraphDialog::on_update() {
	string source = graph->build_source();
	msg_write(source);
	auto s = nix::Shader::create(source);
	if (s) {
		//delete data->appearance.shader;
		data->appearance.shader = s;
		//data->notify();
		mode_material->multi_view->force_redraw();
	}
}

void ShaderGraphDialog::on_right_button_down() {
	popup->open_popup(this);
}

ShaderGraphDialog::HoverData::HoverData() {
	node = nullptr;
	port_in = port_out = param = -1;
}

ShaderGraphDialog::HoverData ShaderGraphDialog::get_hover() {
	HoverData h;

	auto e = hui::GetEvent();
	for (auto *n: graph->nodes) {
		if (node_area(n).inside(e->mx, e->my)) {
			h.node = n;
		}
		for (int i=0; i<n->params.num; i++) {
			if (node_get_in_rect(n, i).inside(e->mx, e->my)) {
				h.node = n;
				h.port_in = i;
			}
			if (node_get_param_rect(n, i).inside(e->mx, e->my)) {
				h.node = n;
				h.param = i;
			}
		}
		for (int i=0; i<n->output.num; i++)
			if (node_get_out_rect(n, i).inside(e->mx, e->my)) {
				h.node = n;
				h.port_out = i;
			}

	}

	return h;
}

ShaderGraphDialog::NewLinkData::NewLinkData() {
	is_source = false;
	node = nullptr;
	port = -1;
}
