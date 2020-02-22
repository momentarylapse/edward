/*
 * ShaderGraphDialog.cpp
 *
 *  Created on: 21.02.2020
 *      Author: michi
 */

#include "ShaderGraphDialog.h"
#include "../ModeMaterial.h"
#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/ColorScheme.h"
#include "../../../Data/Material/DataMaterial.h"
#include "../../../Data/Material/ShaderGraph.h"
#include "../../../lib/math/rect.h"
#include "../../../lib/nix/nix.h"


const int NODE_WIDTH = 170;
const int NODE_HEADER_HEIGHT = 25;
const int NODE_PORT_HEIGHT = 20;


ShaderGraphDialog::ShaderGraphDialog(DataMaterial *_data) {
	data = _data;

	move_dx = move_dy = -1;
	graph = new ShaderGraph();
	//graph->add("Basic", 300, 50);
	//graph->add("Texture", 100, 50);

	auto n1 = graph->add("Texture", 50, 50);
	auto n2 = graph->add("BasicLighting", 250, 100);
	auto n3 = graph->add("Output", 450, 50);
	graph->connect(n1, 0, n2, 0);
	graph->connect(n2, 0, n3, 0);

	from_source("Grid grid '' vertical\n"\
			"\tGrid ? ''\n"
			"\t\tDrawingArea area '' grabfocus expandx expandy\n"\
			"\t\tMultilineEdit source '' disabled\n"\
			"\tGrid bb '' buttonbar\n"\
			"\t\tCheckBox show-source 'Show source'\n"
			"\t\tButton update 'Update'");
	event_xp("area", "hui:draw", [=](Painter *p){ on_draw(p); });
	event_x("area", "hui:mouse-move", [=]{ on_mouse_move(); });
	event_x("area", "hui:left-button-down", [=]{ on_left_button_down(); });
	event_x("area", "hui:left-button-up", [=]{ on_left_button_up(); });
	event_x("area", "hui:right-button-down", [=]{ on_right_button_down(); });
	event_x("area", "hui:key-down", [=]{ on_key_down(); });
	event("update", [=]{ on_update(); });
	event("show-source", [=]{ hide_control("source", !is_checked("")); });

	hide_control("source", true);

	popup = new hui::Menu();
	for (int i=0; i<graph->NODE_TYPES.num; i++) {
		popup->add(graph->NODE_TYPES[i], "add-node-" + i2s(i));
		event("add-node-" + i2s(i), [=]{
			graph->add(graph->NODE_TYPES[i], 400, 200);
		});
	}
	popup->add("Reset", "reset");
	event("reset", [=]{ on_reset(); });

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
	int h = NODE_HEADER_HEIGHT + n->params.num * NODE_PORT_HEIGHT + n->output.num * NODE_PORT_HEIGHT + 8;
	return rect(n->x, n->x + NODE_WIDTH, n->y-5, n->y + h);
}

void ShaderGraphDialog::draw_node(Painter *p, ShaderNode *n) {
	p->set_color(scheme.GRID);
	p->set_roundness(12);
	p->draw_rect(node_area(n));
	p->set_roundness(0);

	p->set_color(scheme.TEXT);
	if (n == hover.node)
		p->set_color(scheme.hoverify(scheme.TEXT));
	p->set_font_size(16);
	p->draw_str(n->x + NODE_WIDTH / 2 - p->get_str_width(n->type) / 2, n->y+3, n->type);

	p->set_font_size(10);

	// in
	foreachi (auto &pp, n->params, i) {
		p->set_color(scheme.TEXT);
		float y = node_get_in_y(n, i);
		p->draw_str(n->x + 10, y-2, pp.name);// + ": " + shader_value_type_to_str(pp.type));
		if (pp.type == ShaderValueType::COLOR) {
			p->set_color(pp.get_color());
			p->set_roundness(6);
			p->draw_rect(node_get_param_rect(n, i));
			p->set_roundness(0);
		}
		p->set_color(scheme.TEXT);
		if (graph->find_source(n, i))
			p->set_color(ColorInterpolate(scheme.TEXT, scheme.GRID, 0.5f));
		if (n == hover.node and i == hover.param)
			p->set_color(scheme.hoverify(scheme.TEXT));
		p->draw_str(n->x + NODE_WIDTH / 2, y-2, pp.value);

		p->set_color(scheme.TEXT);
		if (hover.type == HoverData::Type::PORT_IN and n == hover.node and i == hover.port)
			p->set_color(scheme.hoverify(scheme.TEXT));
		p->draw_circle(n->x - 5, y, 5);
	}

	// out
	foreachi (auto &pp, n->output, i) {
		float y = node_get_out_y(n, i);
		p->set_color(scheme.TEXT);
		p->draw_str(n->x + 40, y-2, "out: " + pp.name);// + ": " + shader_value_type_to_str(pp.type));
		if (hover.type == HoverData::Type::PORT_OUT and n == hover.node and i == hover.port)
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

	if (selection.type == HoverData::Type::PORT_OUT) {
		auto e = hui::GetEvent();
		p->set_color(Red);
		p->draw_line(selection.node->x + NODE_WIDTH + 5, node_get_out_y(selection.node, selection.port), e->mx, e->my);
	}
	if (selection.type == HoverData::Type::PORT_IN) {
		auto e = hui::GetEvent();
		p->set_color(Red);
		p->draw_line(selection.node->x - 5, node_get_in_y(selection.node, selection.port), e->mx, e->my);
	}

	/*p->set_font_size(9);
	p->set_color(scheme.TEXT);
	p->draw_str(600, 10, graph->build_fragment_source());*/
}

void ShaderGraphDialog::on_key_down() {
	auto e = hui::GetEvent();
	if (e->key_code == hui::KEY_DELETE)
		if (selection.node) {
			graph->remove(selection.node);
			selection = HoverData();
			on_update();
		}
	redraw("area");
}

void ShaderGraphDialog::on_left_button_down() {
	auto e = hui::GetEvent();
	hover = get_hover();
	selection = hover;
	if (selection.type == HoverData::Type::PORT_IN) {
		graph->unconnect(nullptr, -1, selection.node, selection.port);
		on_update();
	} else if (selection.type == HoverData::Type::PORT_OUT) {
		graph->unconnect(selection.node, selection.port, nullptr, -1);
		on_update();
	} else if (selection.type == HoverData::Type::PARAMETER) {
		auto &pp = selection.node->params[selection.param];
		if (pp.type == ShaderValueType::FLOAT) {
			if (pp.options.head(6) == "range=") {
				auto xx = pp.options.substr(6,-1).explode(":");
				float _min = xx[0]._float();
				float _max = xx[1]._float();
				rect r = node_get_param_rect(selection.node, selection.port);
				float f = _min + (_max - _min) * (e->mx - r.x1) / r.width();
				pp.value = f2s(f, 3);
			}
		} else if (pp.type == ShaderValueType::COLOR) {
			color col = pp.get_color();
			if (hui::SelectColor(win, col)) {
				pp.set_color(hui::Color);
				on_update();
			}
		}
	} else if (selection.type == HoverData::Type::NODE) {
		move_dx = e->mx - selection.node->x;
		move_dy = e->my - selection.node->y;
	}
	redraw("area");
}

void ShaderGraphDialog::on_left_button_up() {
	if (selection.type == HoverData::Type::PORT_OUT and hover.type == HoverData::Type::PORT_IN) {
		graph->connect(selection.node, selection.port, hover.node, hover.port);
		on_update();
	} else if (selection.type == HoverData::Type::PORT_IN and hover.type == HoverData::Type::PORT_OUT) {
		graph->connect(hover.node, hover.port, selection.node, selection.port);
		on_update();
	} else if (selection.type == HoverData::Type::PARAMETER) {
		auto &pp = selection.node->params[selection.param];
		if (pp.type == ShaderValueType::FLOAT) {
			on_update();
		}
	}
	selection = HoverData();
	redraw("area");
}

void ShaderGraphDialog::on_mouse_move() {
	auto e = hui::GetEvent();
	if (selection.type == HoverData::Type::NODE) {
		selection.node->x = e->mx - move_dx;
		selection.node->y = e->my - move_dy;
	} else if (selection.type == HoverData::Type::PARAMETER) {
		auto &pp = selection.node->params[selection.param];
		if (pp.type == ShaderValueType::FLOAT) {
			if (pp.options.head(6) == "range=") {
				auto xx = pp.options.substr(6,-1).explode(":");
				float _min = xx[0]._float();
				float _max = xx[1]._float();
				rect r = node_get_param_rect(hover.node, hover.port);
				float f = _min + (_max - _min) * clampf((e->mx - r.x1) / r.width(), 0, 1);
				pp.value = f2s(f, 3);
			}
		}
	} else {
		hover = get_hover();
	}
	redraw("area");
}

void ShaderGraphDialog::on_update() {
	string source = graph->build_source();
	set_string("source", graph->build_fragment_source());
	//msg_write(source);
	try {
		auto s = nix::Shader::create(source);
		//delete data->appearance.shader;
		data->appearance.shader = s;
		//data->notify();
		mode_material->multi_view->force_redraw();
	} catch (Exception &e) {
		ed->error_box(e.message());
	}
}

void ShaderGraphDialog::on_reset() {
	graph->clear();
	graph->add("Output", 450, 50);
	on_update();
}

void ShaderGraphDialog::on_right_button_down() {
	popup->open_popup(this);
}

ShaderGraphDialog::HoverData::HoverData() {
	type = Type::NONE;
	node = nullptr;
	port = param = -1;
}

ShaderGraphDialog::HoverData ShaderGraphDialog::get_hover() {
	HoverData h;

	auto e = hui::GetEvent();
	for (auto *n: graph->nodes) {
		if (node_area(n).inside(e->mx, e->my)) {
			h.type = h.Type::NODE;
			h.node = n;
		}
		for (int i=0; i<n->params.num; i++) {
			if (node_get_in_rect(n, i).inside(e->mx, e->my)) {
				h.type = h.Type::PORT_IN;
				h.node = n;
				h.port = i;
			}
			if (node_get_param_rect(n, i).inside(e->mx, e->my)) {
				h.type = h.Type::PARAMETER;
				h.node = n;
				h.param = i;
			}
		}
		for (int i=0; i<n->output.num; i++)
			if (node_get_out_rect(n, i).inside(e->mx, e->my)) {
				h.type = h.Type::PORT_OUT;
				h.node = n;
				h.port = i;
			}

	}

	return h;
}

