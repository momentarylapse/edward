/*
 * ShaderGraph.cpp
 *
 *  Created on: 21.02.2020
 *      Author: michi
 */

#include "ShaderGraph.h"
#include "../../lib/image/color.h"
#include "../../lib/file/msg.h"


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

bool ShaderGraph::has_dependency(ShaderNode *s, ShaderNode *d) const {
	for (auto &l: links)
		if (l.source == s and l.dest == d)
			return true;
	return false;
}

Array<ShaderNode*> ShaderGraph::sorted() const {
	Array<ShaderNode*> snodes = nodes;

	for (int i=0; i<snodes.num; i++)
		for (int j=i+1; j<snodes.num; j++) {
			if (has_dependency(snodes[j], snodes[i])) {
				snodes.swap(i, j);
			}
		}

	return snodes;
}

string sg_build_constant(ShaderNode::Parameter &p) {
	if (p.type == ShaderValueType::INT)
		return p.value;
	if (p.type == ShaderValueType::FLOAT)
		return p.value;
	if (p.type == ShaderValueType::COLOR) {
		color c = p.get_color();
		return format("vec4(%.2f, %.2f, %.2f, %.2f)", c.r, c.g, c.b, c.a);
	}
	return "???";
}

color ShaderNode::Parameter::get_color() const {
	if (type != ShaderValueType::COLOR)
		return Black;
	string s = value.replace("#", "").unhex();
	float r = (float)s[0] / 255.0f;
	float g = (float)s[1] / 255.0f;
	float b = (float)s[2] / 255.0f;
	float a = (float)s[3] / 255.0f;
	return color(a, r, g, b);
}

void ShaderNode::Parameter::set_color(const color &c) {
	if (type != ShaderValueType::COLOR)
		return;
	int i[4];
	c.get_int_argb(i);
	int ii = (i[1] << 24) + (i[2] << 16) + (i[3] << 8) + i[0];
	value = string(&ii, 4).hex(true).replace("0x", "#");
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

string ShaderGraph::build_fragment_source() const {
	string pre = "#version 330 core\n"
			"struct Fog { vec4 color; float density; };\n"
			"struct Material { vec4 ambient, diffusive, specular, emission; float shininess; };\n"
			"struct Light { vec4 color; vec3 pos; float radius, harshness; };\n"
			"uniform Material material;\n"
			"uniform Light light;\n"
			"uniform Fog fog;\n"
			"in vec3 fragmentNormal;\n"
			"in vec2 fragmentTexCoord;\n"
			"in vec3 fragmentPos;\n"
			"out vec4 out_color;\n"
			"uniform sampler2D tex0;\n\n"
			"void main() {\n";

	string _main;

	// sort by dependency...

	Array<TempVar> temps;

	for (auto *n: sorted()) {
		if (n->type == "Output") {
			_main += "\tout_color = " + sg_build_value(this, temps, n, 0) + ";\n";
		} else if (n->type == "Color") {
			string t = create_temp(temps, n, 0);
			_main += "\tvec4 " + t + " = " + sg_build_constant(n->params[0]) + ";\n";
		} else if (n->type == "Texture") {
			string t = create_temp(temps, n, 0);
			_main += "\tvec4 " + t + " = texture(tex0, fragmentTexCoord);\n";
		} else if (n->type == "ColorMultiply") {
			string t = create_temp(temps, n, 0);
			_main += "\tvec4 " + t + " = " + sg_build_value(this, temps, n, 0) + " * " + sg_build_value(this, temps, n, 1) + ";\n";
		}
	}
	return pre + _main + "}\n";
}

string ShaderGraph::build_source() const {
	string pre =
			"<VertexShader>\n"
			"#version 330 core\n"
			"uniform mat4 mat_mvp;\n"
			"uniform mat4 mat_m;\n"
			"uniform mat4 mat_v;\n"
			"layout(location = 0) in vec3 inPosition;\n"
			"layout(location = 1) in vec3 inNormal;\n"
			"layout(location = 2) in vec2 inTexCoord;\n"
			"out vec3 fragmentNormal;\n"
			"out vec2 fragmentTexCoord;\n"
			"out vec3 fragmentPos; // camera space\n"
			"void main() {\n"
			"	gl_Position = mat_mvp * vec4(inPosition,1);\n"
			"	fragmentNormal = (mat_v * mat_m * vec4(inNormal,0)).xyz;\n"
			"	fragmentTexCoord = inTexCoord;\n"
			"	fragmentPos = (mat_v * mat_m * vec4(inPosition,1)).xyz;\n"
			"}\n"
			"</VertexShader>\n"
			"<FragmentShader>\n";
	string post = "</FragmentShader>\n";

	return pre + build_fragment_source() + post;
}

void ShaderGraph::remove(ShaderNode *n) {
	for (int i=links.num-1; i>=0; i--)
		if (links[i].source == n or links[i].dest == n) {
			links.erase(i);
		}
	for (int i=0; i<nodes.num; i++)
		if (nodes[i] == n)
			nodes.erase(i);
	delete n;
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
	unconnect(s, sp, nullptr, -1);
	unconnect(nullptr, -1, d, dp);
	Link l;
	l.source = s;
	l.source_port = sp;
	l.dest = d;
	l.dest_port = dp;
	links.add(l);
}

void ShaderGraph::unconnect(ShaderNode *s, int sp, ShaderNode *d, int dp) {
	foreachi (auto &l, links, i) {
		if (s) {
			if (l.source != s or l.source_port != sp)
				continue;
		}
		if (d) {
			if (l.dest != d or l.dest_port != dp)
				continue;
		}
		links.erase(i);
		break;
	}

}
