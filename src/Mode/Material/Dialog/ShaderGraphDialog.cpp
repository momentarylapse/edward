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
#include "../../../Data/Material/ShaderNode.h"
#include "../../../lib/math/rect.h"
#include "../../../lib/nix/nix.h"
#include "../../../Storage/Storage.h"


const int NODE_WIDTH = 170;
const int NODE_HEADER_HEIGHT = 22;
const int NODE_PORT_HEIGHT = 20;
const float NODE_ROUNDNESS = 8;


const float MIN_VIEW_SCALE = 0.2f;
const float MAX_VIEW_SCALE = 2.0f;

namespace nix{
	extern string shader_error; // -> nix
};

string file_secure(const Path &filename); // -> ModelPropertiesDialog

rect node_area(ShaderNode *n);

bool test_shader_file(const Path &filename) {
	auto *shader = nix::Shader::load(filename);
	msg_todo("TESTME  test_shader_file");
	return shader;
}

class MultiChoice : public hui::Dialog {
public:
	int selected;
	MultiChoice(const Array<string> &choices, hui::Window *parent) : hui::Dialog("choice", 400, 400, parent, false) {
		selected = -1;
		set_options("", "resizable,headerbar");
		add_list_view("!nobar\\a", 0, 0, "choices");
		for (auto &c: choices)
			add_string("choices", c);
		event_x("choices", "hui:activate", [=] {
			selected = get_int("choices");
			request_destroy();
		});
	}
};

ShaderGraphDialog::ShaderGraphDialog(DataMaterial *_data) {
	data = _data;

	mx = my = -1;
	move_dx = move_dy = -1;
	view_scale = 1;
	view_offset_x = 0;
	view_offset_y = 0;
	graph = data->shader.graph;

	from_source("Grid grid '' vertical\n"\
			"\tGrid ? ''\n"
			"\t\tDrawingArea area '' grabfocus expandx expandy\n"\
			"\t\tMultilineEdit source '' disabled\n"\
			"\tGrid bb '' buttonbar\n"\
			"\t\tCheckBox show-source 'Show source'\n"
			"\t\tButton update 'Update'\n"
			"\t\tButton shader-new 'New'\n"
			"\t\tButton shader-load 'Load'\n"
			"\t\tButton shader-save 'Save'\n"
			"\t\tButton shader-default 'Default'\n");
	event_xp("area", "hui:draw", [=](Painter *p){ on_draw(p); });
	event_x("area", "hui:mouse-move", [=]{ on_mouse_move(); });
	event_x("area", "hui:mouse-wheel", [=]{ on_mouse_wheel(); });
	event_x("area", "hui:left-button-down", [=]{ on_left_button_down(); });
	event_x("area", "hui:left-button-up", [=]{ on_left_button_up(); });
	event_x("area", "hui:right-button-down", [=]{ on_right_button_down(); });
	event_x("area", "hui:key-down", [=]{ on_key_down(); });
	event("update", [=]{ on_update(); });
	event("show-source", [=]{ hide_control("source", !is_checked("")); });
	event("shader-new", [=]{
		auto dlg = new MultiChoice({"default", "pure color out", "cube map"}, win);
		dlg->run();
		int sel = dlg->selected;
		delete dlg;
		if (sel == 0) {
			graph->make_default_for_engine();
		} else if (sel == 1) {
			graph->make_default_basic();
		} else if (sel == 2) {
			graph->make_default_cube_map();
		} else {
			return;
		}
		data->shader.file = "";
		data->shader.is_default = false;
		data->reset_history(); // TODO: actions
		request_optimal_view();
		data->notify(data->MESSAGE_CHANGE);
	});
	event("shader-default", [=]{
		data->shader.set_engine_default();
		data->reset_history(); // TODO: actions
		data->notify(data->MESSAGE_CHANGE);
	});
	event("shader-load", [=]{
		if (storage->file_dialog(FD_SHADERFILE,false,true)) {
			if (test_shader_file(storage->dialog_file)) {
				data->shader.file = storage->dialog_file;
				data->shader.load_from_file();
				request_optimal_view();
				data->notify(data->MESSAGE_CHANGE);
			} else {
				ed->error_box(_("Error in shader file:\n") + nix::shader_error);
			}
		}
	});
	event("shader-save", [=]{
		if (storage->file_dialog(FD_SHADERFILE,true,true)) {
			data->shader.file = storage->dialog_file;
			data->shader.save_to_file();
		}
	});

	hide_control("source", true);

	popup = new hui::Menu();
	hui::Menu *sub = nullptr;
	auto types = graph->enumerate();
	for (int i=0; i<types.num; i++) {
		string s = types[i];
		if (s[0] == '-') {
			sub = new hui::Menu();
			popup->add_sub_menu(s.replace("-", ""), "", sub);
		} else {
			sub->add(types[i], "add-node-" + i2s(i));
			event("add-node-" + i2s(i), [=]{
				graph->add(types[i], 400, 200);
			});
		}
	}
	popup->add("Reset", "reset");
	event("reset", [=]{ on_reset(); });

	_optimal_view_requested = true;
}

ShaderGraphDialog::~ShaderGraphDialog() {
}

void ShaderGraphDialog::request_optimal_view() {
	_optimal_view_requested = true;
	redraw("area");
}

void ShaderGraphDialog::_optimize_view(const rect &area) {
	rect ga = rect(1000,-1000,1000,-1000);
	for (auto *n: weak(graph->nodes)) {
		auto a = node_area(n);
		ga.x1 = min(ga.x1, a.x1);
		ga.y1 = min(ga.y1, a.y1);
		ga.x2 = max(ga.x2, a.x2);
		ga.y2 = max(ga.y2, a.y2);
	}
	view_scale = clamp(min(area.width() / ga.width(), area.height() / ga.height()), MIN_VIEW_SCALE, MAX_VIEW_SCALE);
	view_offset_x = ga.mx() - area.mx() / view_scale;
	view_offset_y = ga.my() - area.my() / view_scale;
	//virt = (phys / view_scale) + view_offset_x;
	_optimal_view_requested = false;
}

float ShaderGraphDialog::proj_x(float x) {
	return (x - view_offset_x) * view_scale;
}

float ShaderGraphDialog::proj_y(float y) {
	return (y - view_offset_y) * view_scale;
}

float node_get_in_y(ShaderNode *n, int i) {
	return n->y + NODE_HEADER_HEIGHT + i * NODE_PORT_HEIGHT + NODE_PORT_HEIGHT/2;
}

float node_get_out_y(ShaderNode *n, int i) {
	return n->y + NODE_HEADER_HEIGHT + /*n->params.num * NODE_PORT_HEIGHT*/ + i * NODE_PORT_HEIGHT + NODE_PORT_HEIGHT/2;
}

rect node_get_param_rect(ShaderNode *n, int i, float fraction=1) {
	int y = node_get_in_y(n, i);
	float x0 = n->x + 30;
	float x1 = n->x + NODE_WIDTH - 30;
	return rect(x0, x0 + (x1 - x0) * fraction, y-8, y+8);
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
	int h = NODE_HEADER_HEIGHT + max(n->params.num, n->output.num) * NODE_PORT_HEIGHT + 8;
	return rect(n->x, n->x + NODE_WIDTH, n->y-5, n->y + h);
}

rect node_header_area(ShaderNode *n) {
	int h = NODE_HEADER_HEIGHT;
	return rect(n->x, n->x + NODE_WIDTH, n->y-5, n->y + h);
}

bool node_in_pluggable(ShaderNode *n, int i) {
	if (n->params[i].type == ShaderValueType::LITERAL)
		return false;
	if (n->params[i].type == ShaderValueType::INT and (n->params[i].options.head(7) == "choice="))
		return false;
	return true;
}

string short_port_name(const string &n) {
	if (n == "red")
		return "r";
	if (n == "green")
		return "g";
	if (n == "blue")
		return "b";
	if (n == "alpha")
		return "a";
	if (n == "pos")
		return "p";
	if (n == "normal" or n == "normals")
		return "n";
	if (n == "out" or n == "value" or n == "vector" or n == "color")
		return "";
	return n.head(2);
}

void draw_node_param(Painter *p, ShaderGraphDialog *dlg, ShaderNode *n, ShaderNode::Parameter &pp, int i, float yt) {

	color bg = color::interpolate(scheme.BACKGROUND, scheme.GRID, 0.5f);
	if (pp.type == ShaderValueType::COLOR) {
		bg = pp.get_color();
	}
	p->set_color(bg);
	p->set_roundness(3);
	p->draw_rect(node_get_param_rect(n, i));
	p->set_roundness(0);
	if (pp.type == ShaderValueType::FLOAT) {
		if (pp.options.head(6) == "range=") {
			auto xx = pp.options.substr(6,-1).explode(":");
			float _min = xx[0]._float();
			float _max = xx[1]._float();
			float scale = clamp((pp.value._float() - _min) / (_max - _min), 0.0f, 1.0f);
			p->set_color(scheme.BACKGROUND);
			p->set_roundness(3);
			p->draw_rect(node_get_param_rect(n, i, scale));
			p->set_roundness(0);
		}
	}
	p->set_color(scheme.TEXT);
	if (dlg->graph->find_source(n, i))
		p->set_color(color::interpolate(scheme.TEXT, scheme.GRID, 0.5f));
	if (n == dlg->hover.node and i == dlg->hover.param)
		p->set_color(scheme.hoverify(scheme.TEXT));

	string value = pp.value;
	if (pp.type == ShaderValueType::INT) {
		if (pp.options.head(7) == "choice=") {
			auto xx = pp.options.substr(7, -1).explode("|");
			int n = clamp(value._int(), 0, xx.num - 1);
			value = xx[n];
		}
	}
	p->draw_str(n->x + NODE_WIDTH / 2 - p->get_str_width(value)/2, yt, value);
}

void ShaderGraphDialog::draw_node(Painter *p, ShaderNode *n) {
	color bg = scheme.GRID;
	p->set_color(bg);
	p->set_roundness(NODE_ROUNDNESS);
	p->draw_rect(node_area(n));

	// header
	int h = n->type.hash();
	bg = color::interpolate(scheme.GRID, color::hsb(loop(h/7.0f, 0.0f, 1.0f), 0.8f, 0.3f, 1), 0.3f);
	if (n == hover.node)
		bg = scheme.hoverify(bg);
	p->set_color(bg);
	p->draw_rect(node_header_area(n));
	p->set_roundness(0);

	p->set_color(scheme.TEXT);
	p->set_font_size(12);
	p->draw_str(n->x + NODE_WIDTH / 2 - p->get_str_width(n->type) / 2, n->y+3, n->type);

	p->set_font_size(10);

	// in
	foreachi (auto &pp, n->params, i) {
		p->set_color(scheme.TEXT);
		float y = node_get_in_y(n, i);
		float yt = y - 4;
		p->draw_str(n->x + 10, yt, short_port_name(pp.name));

		if (!graph->find_source(n, i))
			draw_node_param(p, this, n, pp, i, yt);

		p->set_color(scheme.TEXT);
		if (hover.type == HoverData::Type::PORT_IN and n == hover.node and i == hover.port)
			p->set_color(scheme.hoverify(scheme.TEXT));
		if (node_in_pluggable(n, i))
			p->draw_circle(n->x - 5, y, 5);
	}

	// out
	foreachi (auto &pp, n->output, i) {
		float y = node_get_out_y(n, i);
		float yt = y - 4;
		p->set_color(scheme.TEXT);
		float w = p->get_str_width(short_port_name(pp.name));
		p->draw_str(n->x + NODE_WIDTH - 10 - w, yt, short_port_name(pp.name));
		if (hover.type == HoverData::Type::PORT_OUT and n == hover.node and i == hover.port)
			p->set_color(scheme.hoverify(scheme.TEXT));
		p->draw_circle(n->x + NODE_WIDTH + 5, y, 5);
	}
}

void ShaderGraphDialog::update_mouse() {
	auto e = hui::GetEvent();
	mx = (e->mx / view_scale) + view_offset_x;
	my = (e->my / view_scale) + view_offset_y;
}


void draw_arrow(Painter *p, const complex &m, const complex &_d, float length) {
	complex d = _d / _d.abs();
	complex e = d * complex::I;
	Array<complex> pp;
	pp.add(m + d * length);
	pp.add(m - d * length + e * length / 2);
	pp.add(m - d * length * 0.8f);
	pp.add(m - d * length - e * length / 2);
	p->draw_polygon(pp);
}

void ShaderGraphDialog::draw_cable(Painter *p, ShaderNode *source, int source_port, ShaderNode *dest, int dest_port) {
	p->set_color(Blue);
	p->set_line_width(2);

	complex p0 = complex(source->x + NODE_WIDTH + 5, node_get_out_y(source, source_port));
	complex p1 = complex(dest->x - 5, node_get_in_y(dest, dest_port));

	float length = (p1 - p0).abs();
	Interpolator<complex> inter(Interpolator<complex>::TYPE_CUBIC_SPLINE);
	inter.add2(p0, complex(length,0));
	inter.add2(p1, complex(length,0));

	p->set_line_dash({5, 2}, 0);
	Array<complex> cc;
	for (float t=0; t<=1.0f; t+=0.025f)
		cc.add(inter.get(t));
	p->draw_lines(cc);
	p->set_line_dash({}, 0);

	draw_arrow(p, inter.get(0.5f), inter.getTang(0.5f), min(length / 7, 14.0f));
}

void ShaderGraphDialog::on_draw(Painter *p) {
	//int w = p->width;
	//int h = p->height;
	if (_optimal_view_requested)
		_optimize_view(p->area());
	p->set_color(scheme.BACKGROUND);
	p->draw_rect(p->area());

	float m[4] = {view_scale, 0, 0, view_scale};
	p->set_transform(m, complex(-view_offset_x, -view_offset_y)*view_scale);

	if (data->shader.from_graph and !data->shader.is_default) {
		for (auto *n: weak(graph->nodes))
			draw_node(p, n);

		for (auto &l: graph->links)
			draw_cable(p, l.source, l.source_port, l.dest, l.dest_port);

		if (selection.type == HoverData::Type::PORT_OUT) {
			auto e = hui::GetEvent();
			p->set_color(Red);
			p->draw_line(selection.node->x + NODE_WIDTH + 5, node_get_out_y(selection.node, selection.port), mx, my);
		}
		if (selection.type == HoverData::Type::PORT_IN) {
			auto e = hui::GetEvent();
			p->set_color(Red);
			p->draw_line(selection.node->x - 5, node_get_in_y(selection.node, selection.port), mx, my);
		}
	}

	float m_id[4] = {1, 0, 0, 1};
	p->set_transform(m_id, complex(0, 0));
	p->set_font_size(9);
	p->set_color(Black);
	if (data->shader.is_default) {
		p->draw_str(10, 10, "- engine default -");
	} else {
		p->draw_str(10, 10, file_secure(data->shader.file.str()));
		if (!data->shader.from_graph) {
			p->set_color(Red);
			p->draw_str(10, 20, "not from graph!");
		}
	}
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

class TextInputDialog : public hui::Dialog {
public:
	TextInputDialog(hui::Window *parent, const string &orig) : hui::Dialog(_("Text"), 400, 100, parent, false) {
		reply = orig;
		from_source("Grid ? '' vertical\n"
		"	Edit text '' expandx\n"
		"	Grid ? '' buttonbar\n"
		"		Button ok 'OK' default");
		set_string("text", orig);
		event("ok", [=]{
			reply = get_string("text");
			request_destroy();
		});
	}
	string reply;
};

string text_input_dialog(hui::Window *parent, const string &orig) {
	auto dlg = new TextInputDialog(parent, orig);
	dlg->run();
	string r = dlg->reply;
	delete dlg;
	return r;
}

void ShaderGraphDialog::on_left_button_down() {
	update_mouse();

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
				float f = _min + (_max - _min) * (mx - r.x1) / r.width();
				pp.value = f2s(f, 3);
			}
		} else if (pp.type == ShaderValueType::COLOR) {
			color col = pp.get_color();
			if (hui::SelectColor(win, col)) {
				pp.set_color(hui::Color);
				on_update();
			}
		} else if (pp.type == ShaderValueType::INT) {
			if (pp.options.head(7) == "choice=") {
				auto xx = pp.options.substr(7, -1).explode("|");
				pp.value = i2s(loop(pp.value._int() + 1, 0, xx.num - 1));
				on_update();
			}
		} else if (pp.type == ShaderValueType::LITERAL) {
			pp.value = text_input_dialog(win, pp.value);
			on_update();
		}
	} else if (selection.type == HoverData::Type::NODE) {
		move_dx = mx - selection.node->x;
		move_dy = my - selection.node->y;
	} else if (selection.type == HoverData::Type::NONE) {
		move_dx = mx;
		move_dy = my;
		selection.type = HoverData::Type::VIEW;
	}
	redraw("area");
}

void ShaderGraphDialog::on_left_button_up() {
	try {
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
	} catch (Exception &e) {
		ed->error_box(e.message());
	}
}

void ShaderGraphDialog::on_mouse_move() {
	update_mouse();

	if (selection.type == HoverData::Type::NODE) {
		selection.node->x = mx - move_dx;
		selection.node->y = my - move_dy;
	} else if (selection.type == HoverData::Type::PARAMETER) {
		auto &pp = selection.node->params[selection.param];
		if (pp.type == ShaderValueType::FLOAT) {
			if (pp.options.head(6) == "range=") {
				auto xx = pp.options.substr(6,-1).explode(":");
				float _min = xx[0]._float();
				float _max = xx[1]._float();
				rect r = node_get_param_rect(hover.node, hover.port);
				float f = _min + (_max - _min) * clamp((mx - r.x1) / r.width(), 0.0f, 1.0f);
				pp.value = f2s(f, 3);
			}
		}
	} else if (selection.type == HoverData::Type::VIEW) {
		auto e = hui::GetEvent();
		view_offset_x = move_dx - (e->mx / view_scale);
		view_offset_y = move_dy - (e->my / view_scale);
	} else {
		hover = get_hover();
	}
	redraw("area");
}

void ShaderGraphDialog::on_mouse_wheel() {
	auto e = hui::GetEvent();
	view_scale = clamp(view_scale * exp(e->scroll_y * 0.05f), MIN_VIEW_SCALE, MAX_VIEW_SCALE);

	view_offset_x = mx -  e->mx / view_scale;
	view_offset_y = my -  e->my / view_scale;

	redraw("area");
}

void ShaderGraphDialog::on_update() {
	string source = graph->build_source();
	data->shader.code = source;
	set_string("source", source);

	data->shader.from_graph = true;
	data->shader.is_default = false;
	data->reset_history(); // TODO: actions
	data->notify(data->MESSAGE_CHANGE);
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

	for (auto *n: weak(graph->nodes)) {
		if (node_area(n).inside(mx, my)) {
			h.type = h.Type::NODE;
			h.node = n;
		}
		for (int i=0; i<n->params.num; i++) {
			if (node_get_in_rect(n, i).inside(mx, my) and node_in_pluggable(n, i)) {
				h.type = h.Type::PORT_IN;
				h.node = n;
				h.port = i;
			}
			if (node_get_param_rect(n, i).inside(mx, my) and !graph->find_source(n, i)) {
				h.type = h.Type::PARAMETER;
				h.node = n;
				h.param = i;
			}
		}
		for (int i=0; i<n->output.num; i++)
			if (node_get_out_rect(n, i).inside(mx, my)) {
				h.type = h.Type::PORT_OUT;
				h.node = n;
				h.port = i;
			}

	}

	return h;
}

