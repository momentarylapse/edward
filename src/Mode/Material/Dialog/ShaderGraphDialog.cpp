/*
 * ShaderGraphDialog.cpp
 *
 *  Created on: 21.02.2020
 *      Author: michi
 */

#include "ShaderGraphDialog.h"
#include "../../../MultiView/ColorScheme.h"
#include "../../../lib/math/rect.h"
#include "../../../Data/Material/ShaderGraph.h"


const int NODE_WIDTH = 150;
const int NODE_HEADER_HEIGHT = 25;
const int NODE_PORT_HEIGHT = 20;

ShaderGraphDialog::ShaderGraphDialog(hui::Window *parent, DataMaterial *_data) :
	hui::Dialog("Shader Graph", 1000, 600, parent, true)
{
	data = _data;

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

	add_drawing_area("", 0, 0, "area");

	/*from_source("Grid grid '' vertical"\
			"\tDrawingArea area '' grabfocus expandx expandy"\
			"\tButton update 'Update'");*/
	event_xp("area", "hui:draw", [=](Painter *p){ on_draw(p); });
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

void ShaderGraphDialog::draw_node(Painter *p, ShaderNode *n) {
	p->set_color(scheme.GRID);
	p->draw_rect(rect(n->x, n->x + NODE_WIDTH, n->y, n->y + 200));

	p->set_color(scheme.TEXT);
	p->set_font_size(15);
	p->draw_str(n->x + NODE_WIDTH / 2 - p->get_str_width(n->type) / 2, n->y+3, n->type);

	p->set_font_size(11);
	foreachi (auto &pp, n->params, i) {
		p->set_color(scheme.TEXT);
		float y = node_get_in_y(n, i);
		p->draw_str(n->x + 20, y, pp.name);// + ": " + shader_value_type_to_str(pp.type));
		if (graph->find_source(n, i))
			p->set_color(ColorInterpolate(scheme.TEXT, scheme.GRID, 0.5f));
		p->draw_str(n->x + NODE_WIDTH / 2, y, pp.value);
	}
	p->set_color(scheme.TEXT);
	foreachi (auto &pp, n->output, i) {
		float y = node_get_out_y(n, i);
		p->draw_str(n->x + 40, y, "out: " + pp.name);// + ": " + shader_value_type_to_str(pp.type));
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

	p->set_font_size(11);
	p->set_color(scheme.TEXT);
	p->draw_str(600, 10, graph->build());
}

void ShaderGraphDialog::on_left_button_down() {
}

void ShaderGraphDialog::on_left_button_up() {
}

void ShaderGraphDialog::on_mouse_move() {
}
