/*
 * ShaderGraph.cpp
 *
 *  Created on: 21.02.2020
 *      Author: michi
 */

#include "ShaderGraph.h"


string shader_value_type_to_str(ShaderValueType t) {
	if (t == ShaderValueType::INT)
		return "int";
	if (t == ShaderValueType::FLOAT)
		return "float";
	if (t == ShaderValueType::COLOR)
		return "color";
	if (t == ShaderValueType::VEC3)
		return "vec3";
	return "???";
}


void _build_node(ShaderNode *n) {
	if (n->type == "Color") {
		n->params.add({ShaderValueType::COLOR, "Value", "#ffffffff"});
		n->output.add({ShaderValueType::COLOR, "Value"});
	} else if (n->type == "Output") {
		n->params.add({ShaderValueType::COLOR, "Color", "#ff0000ff"});
	} else if (n->type == "Texture") {
		n->output.add({ShaderValueType::COLOR, "Value"});
	} else if (n->type == "ColorMultiply") {
		n->params.add({ShaderValueType::COLOR, "a", "#ffffffff"});
		n->params.add({ShaderValueType::COLOR, "b", "#ffffffff"});
		n->output.add({ShaderValueType::COLOR, "out"});
	}
}

ShaderGraph::ShaderGraph() {
}

ShaderGraph::~ShaderGraph() {
}

ShaderNode::ShaderNode(const string &_type, int _x, int _y) {
	type = _type;
	x = _x;
	y = _y;

	_build_node(this);
}

string sg_build_constant(ShaderNode::Parameter &p) {
	if (p.type == ShaderValueType::INT)
		return p.value;
	if (p.type == ShaderValueType::FLOAT)
		return p.value;
	if (p.type == ShaderValueType::COLOR) {
		string s = p.value.substr(1, -1).unhex();
		float r = (float)s[0] / 255.0f;
		float g = (float)s[1] / 255.0f;
		float b = (float)s[2] / 255.0f;
		float a = (float)s[3] / 255.0f;
		return format("vec4(%.2f, %.2f, %.2f, %.2f)", r, g, b, a);
	}
	return "???";
}


struct TempVar {
	string name;
	ShaderNode *source;
	int port;
};

string create_temp(Array<TempVar> &temps, ShaderNode *source, int port) {
	TempVar t;
	t.name = format("tmp%d", temps.num + 1);
	t.source = source;
	t.port = port;
	temps.add(t);
	return t.name;
}

string find_temp(Array<TempVar> &temps, ShaderNode *source, int port) {
	for (auto &t: temps)
		if (t.source == source and t.port == port)
			return t.name;
	return "???";
}

string sg_build_value(const ShaderGraph *g, Array<TempVar> &temps, ShaderNode *n, int i) {
	auto l = g->find_source(n, i);
	if (l)
		return find_temp(temps, l->source, l->source_port);
	return sg_build_constant(n->params[i]);
}

string ShaderGraph::build() const {
	string pre = "#version 450\n"\
			"uniform sampler2D tex;\n"\
			"layout(location = 0) in vec4 in_pos;\n"\
			"layout(location = 1) in vec2 in_tex_coord;\n"\
			"layout(location = 2) in vec3 in_normal;\n"\
			"layout(location = 0) out vec4 out_color;\n\n"\
			"void main() {\n";

	string _main;

	// sort by dependency...

	Array<TempVar> temps;

	for (auto *n: nodes) {
		if (n->type == "Output") {
			_main += "\tout_color = " + sg_build_value(this, temps, n, 0) + ";\n";
		} else if (n->type == "Color") {
			string t = create_temp(temps, n, 0);
			_main += "\tvec4 " + t + " = " + sg_build_constant(n->params[0]) + ";\n";
		} else if (n->type == "Texture") {
			string t = create_temp(temps, n, 0);
			_main += "\tvec4 " + t + " = texture(tex, in_tex_coord);\n";
		} else if (n->type == "ColorMultiply") {
			string t = create_temp(temps, n, 0);
			_main += "\tvec4 " + t + " = " + sg_build_value(this, temps, n, 0) + " * " + sg_build_value(this, temps, n, 1) + ";\n";
		}
	}
	return pre + _main + "}\n";
}

ShaderGraph::Link *ShaderGraph::find_source(ShaderNode *d, int dp) const {
	for (auto &l: links)
		if (l.dest == d and l.dest_port == dp)
			return &l;
	return nullptr;
}

ShaderNode* ShaderGraph::add(const string &type, int x, int y) {
	auto n = new ShaderNode(type, x, y);
	nodes.add(n);
	return n;
}

void ShaderGraph::connect(ShaderNode *s, int sp, ShaderNode *d, int dp) {
	Link l;
	l.source = s;
	l.source_port = sp;
	l.dest = d;
	l.dest_port = dp;
	links.add(l);
}
