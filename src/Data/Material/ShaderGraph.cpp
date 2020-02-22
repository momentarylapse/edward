/*
 * ShaderGraph.cpp
 *
 *  Created on: 21.02.2020
 *      Author: michi
 */

#include "ShaderGraph.h"
#include "../../lib/base/set.h"
#include "../../lib/image/color.h"
#include "../../lib/file/msg.h"
#include "../../lib/xfile/xml.h"


const Array<string> ShaderGraph::NODE_TYPES = {"Color", "Vector", "Texture", "Position", "Normals", "UV", "MultiplyColor", "Output", "BasicLighting", "RandomFloat", "RandomColor", "ColorRed", "RescaleVector", "RescaleVector2"};

string shader_value_type_to_str(ShaderValueType t) {
	if (t == ShaderValueType::INT)
		return "int";
	if (t == ShaderValueType::FLOAT)
		return "float";
	if (t == ShaderValueType::COLOR)
		return "color";
	if (t == ShaderValueType::VEC3)
		return "vec3";
	if (t == ShaderValueType::VEC2)
		return "vec2";
	return "???";
}


void _build_node(ShaderNode *n) {
	if (n->type == "Color") {
		n->params.add({ShaderValueType::COLOR, "value", "#ffffffff"});
		n->output.add({ShaderValueType::COLOR, "value"});
	} else if (n->type == "Vector") {
		n->params.add({ShaderValueType::FLOAT, "x", "0.0", "range=-1:1"});
		n->params.add({ShaderValueType::FLOAT, "y", "0.0", "range=-1:1"});
		n->params.add({ShaderValueType::FLOAT, "z", "1.0", "range=-1:1"});
		n->output.add({ShaderValueType::VEC3, "value"});
	} else if (n->type == "Output") {
		n->params.add({ShaderValueType::COLOR, "color", "#ff0000ff"});
	} else if (n->type == "Texture") {
		n->params.add({ShaderValueType::VEC2, "uv", "-mesh-"});
		n->output.add({ShaderValueType::COLOR, "color"});
		n->dependencies.add("texture");
		n->dependencies.add("uv");
	} else if (n->type == "Normals") {
		n->output.add({ShaderValueType::VEC3, "normals"});
		n->dependencies.add("normal");
	} else if (n->type == "UV") {
		n->output.add({ShaderValueType::VEC2, "uv"});
		n->dependencies.add("uv");
	} else if (n->type == "Position") {
		n->output.add({ShaderValueType::VEC3, "pos"});
		n->dependencies.add("pos");
	} else if (n->type == "MultiplyColor") {
		n->params.add({ShaderValueType::COLOR, "a", "#ffffffff"});
		n->params.add({ShaderValueType::COLOR, "b", "#ffffffff"});
		n->output.add({ShaderValueType::COLOR, "out"});
	} else if (n->type == "ColorRed") {
		n->params.add({ShaderValueType::COLOR, "in", "#ffffffff"});
		n->output.add({ShaderValueType::FLOAT, "out"});
	} else if (n->type == "RandomColor") {
		n->output.add({ShaderValueType::COLOR, "out"});
		n->dependencies.add("rand");
		n->dependencies.add("noise2d");
		n->dependencies.add("uv");
	} else if (n->type == "RandomFloat") {
		n->output.add({ShaderValueType::FLOAT, "out"});
		n->dependencies.add("rand");
		n->dependencies.add("noise2d");
		n->dependencies.add("uv");
	} else if (n->type == "RescaleVector") {
		n->params.add({ShaderValueType::VEC3, "in", "vec3(0,0,0)"});
		n->params.add({ShaderValueType::FLOAT, "scale", "1.0", "range=-2:2"});
		n->params.add({ShaderValueType::FLOAT, "offset", "0.0", "range=-10:10"});
		n->output.add({ShaderValueType::VEC3, "out"});
	} else if (n->type == "RescaleVector2") {
		n->params.add({ShaderValueType::VEC2, "in", "vec2(0,0)"});
		n->params.add({ShaderValueType::FLOAT, "scale", "1.0", "range=-2:2"});
		n->params.add({ShaderValueType::FLOAT, "offset", "0.0", "range=-10:10"});
		n->output.add({ShaderValueType::VEC2, "out"});
	} else if (n->type == "BasicLighting") {
		n->params.add({ShaderValueType::COLOR, "diffuse", "#ffffffff"});
		n->params.add({ShaderValueType::FLOAT, "ambient", "0.3", "range=0:1"});
		n->params.add({ShaderValueType::FLOAT, "specular", "0.1", "range=0:1"});
		n->params.add({ShaderValueType::FLOAT, "shininess", "10.0", "range=0:50"});
		n->params.add({ShaderValueType::COLOR, "emission", "#000000ff"});
		n->params.add({ShaderValueType::VEC3, "normals", "-mesh-"});
		n->output.add({ShaderValueType::COLOR, "out"});
		n->dependencies.add("basic_lighting");
		n->dependencies.add("light");
		n->dependencies.add("material");
		n->dependencies.add("pos");
		n->dependencies.add("normal");
	}
}

ShaderGraph::ShaderGraph() {
}

ShaderGraph::~ShaderGraph() {
	clear();
}

void ShaderGraph::clear() {
	links.clear();
	for (auto *n: nodes)
		delete n;
	nodes.clear();
}

void ShaderGraph::make_default() {
	clear();

	auto n1 = add("Texture", 50, 50);
	auto n2 = add("BasicLighting", 250, 100);
	auto n3 = add("Output", 450, 50);
	connect(n1, 0, n2, 0);
	connect(n2, 0, n3, 0);
}

int sg_node_index(ShaderGraph *g, ShaderNode *n) {
	foreachi (auto *nn, g->nodes, i)
		if (nn == n)
			return i;
	return -1;
}

// who needs sanity checks?!?!?
void ShaderGraph::load(const string &filename) {
	clear();
	msg_write("loading graph..." + filename);

	xml::Parser p;
	p.load(filename);
	if (p.elements.num < 0)
		return;
	for (auto &e: p.elements[0].elements[0].elements) {
		string type = e.value("type");
		int x = e.value("x")._int();
		int y = e.value("y")._int();
		auto *n = new ShaderNode(type, x, y);
		for (auto &pp: n->params)
			pp.value = e.value(pp.name);
		nodes.add(n);
	}
	for (auto &e: p.elements[0].elements[1].elements) {
		Link l;
		l.source = nodes[e.value("source")._int()];
		l.source_port = e.value("sourceport")._int();
		l.dest = nodes[e.value("dest")._int()];
		l.dest_port = e.value("destport")._int();
		links.add(l);
	}
}

void ShaderGraph::save(const string &filename) {
	msg_write("saving graph... " + filename);
	xml::Parser p;
	xml::Element root = {"ShaderGraph"};
	xml::Element enodes = {"Nodes"};
	for (auto *n: nodes) {
		xml::Element e = {"Node"};
		e.add_attribute("type", n->type);
		e.add_attribute("x", i2s(n->x));
		e.add_attribute("y", i2s(n->y));
		for (auto &p: n->params) {
			e.add_attribute(p.name, p.value);
		}
		enodes.add(e);
	}
	xml::Element elinks = {"Links"};
	for (auto &l: links) {
		xml::Element e = xml::Element("Link");
		e.add_attribute("source", i2s(sg_node_index(this, l.source)));
		e.add_attribute("sourceport", i2s(l.source_port));
		e.add_attribute("dest", i2s(sg_node_index(this, l.dest)));
		e.add_attribute("destport", i2s(l.dest_port));
		elinks.add(e);
	}
	root.add(enodes);
	root.add(elinks);
	p.elements.add(root);
	p.save(filename);
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

int get_dep_depth(const ShaderGraph *g, ShaderNode *n, int level) {
	// detect loops
	if (level > g->nodes.num)
		return -1;
	// recursion
	for (auto *m: g->nodes)
		if (g->has_dependency(m, n))
			return get_dep_depth(g, m, level + 1);
	return level;
}

Array<ShaderNode*> ShaderGraph::sorted() const {
	Array<ShaderNode*> snodes;

	/*Array<int> dep_depth;
	for (auto *n: nodes)
		dep_depth.add(get_dep_depth(this, n, 0));
	msg_write(ia2s(dep_depth));*/

	for (int l=0; l<nodes.num; l++) {
		for (auto *n: nodes) {
			int ll = get_dep_depth(this, n, 0);
			if (ll == l)
				snodes.add(n);
		}
	}



	for (int i=0; i<snodes.num; i++)
		for (int j=i+1; j<snodes.num; j++) {
			if (has_dependency(snodes[j], snodes[i])) {
				msg_error("failed to sort");
			}
		}

	return snodes;
}

string sg_build_constant(ShaderNode::Parameter &p) {
	if (p.type == ShaderValueType::INT) {
		return p.value;
	} else if (p.type == ShaderValueType::FLOAT) {
		return p.value;
	} else if (p.type == ShaderValueType::COLOR) {
		color c = p.get_color();
		return format("vec4(%.2f, %.2f, %.2f, %.2f)", c.r, c.g, c.b, c.a);
	} else if (p.type == ShaderValueType::VEC2) {
		return p.value;
	} else if (p.type == ShaderValueType::VEC3) {
		return p.value;
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
	ShaderValueType type;
};

string create_temp(Array<TempVar> &temps, ShaderNode *source, int port) {
	TempVar t;
	t.name = format("tmp%d", temps.num + 1);
	t.source = source;
	t.port = port;
	t.type = source->output[port].type;
	temps.add(t);
	return t.name;
}

string cast(const string &v, ShaderValueType ts, ShaderValueType td) {
	if (ts == ShaderValueType::COLOR and td == ShaderValueType::VEC3)
		return v + ".xyz";
	if (ts == ShaderValueType::VEC3 and td == ShaderValueType::COLOR)
		return "vec4(" + v + ", 1.0)";
	return v;
}

bool can_cast(ShaderValueType ts, ShaderValueType td) {
	if (ts == td)
		return true;
	if (ts == ShaderValueType::COLOR and td == ShaderValueType::VEC3)
		return true;
	if (ts == ShaderValueType::VEC3 and td == ShaderValueType::COLOR)
		return true;
	return false;
}

string find_temp(Array<TempVar> &temps, ShaderNode *source, int port, ShaderValueType wanted_type) {
	for (auto &t: temps)
		if (t.source == source and t.port == port)
			return cast(t.name, t.type, wanted_type);
	return "???";
}

string sg_build_value(const ShaderGraph *g, Array<TempVar> &temps, ShaderNode *n, int i, const string &internal = "") {
	auto l = g->find_source(n, i);
	if (l)
		return find_temp(temps, l->source, l->source_port, n->params[i].type);
	if (internal != "")
		return internal;
	return sg_build_constant(n->params[i]);
}

string build_helper_vars(const Set<string> &vars) {
	string source;
	if (vars.contains("light")) {
		source +=
			"struct Light { vec4 color; vec3 pos; float radius, harshness; };\n"
			"uniform Light light;\n";
	}
	if (vars.contains("fog")) {
		source +=
			"struct Fog { vec4 color; float density; };\n"
			"uniform Fog fog;\n";
	}
	if (vars.contains("material")) {
		source +=
			"struct Material { vec4 ambient, diffusive, specular, emission; float shininess; };\n"
			"uniform Material material;\n";
	}
	if (vars.contains("normal")) {
		source +=
			"layout(location = 0) in vec3 in_normal;\n";
	}
	if (vars.contains("uv")) {
		source +=
			"layout(location = 1) in vec2 in_uv;\n";
	}
	if (vars.contains("pos")) {
		source +=
			"layout(location = 2) in vec3 in_pos;\n";
	}
	if (vars.contains("texture")) {
		source +=
			"uniform sampler2D tex0;\n";
	}
	return source;
}

string build_helper_functions(const Set<string> &funcs) {
	string source;
	if (funcs.contains("rand")) {
		source += "\nfloat rand(vec2 st) {\n"
		"	return fract(sin(dot(st.xy ,vec2(12.9898,78.233))) * 43758.5453);\n"
		"}\n";
	}
	if (funcs.contains("noise2d")) {
		source += "\nfloat noise2d(vec2 p) {\n"
		"	vec2 i = floor(p*10);\n"
		"	vec2 f = fract(p*10);\n"
		"	return mix(mix(rand(i), rand(i + vec2(1.0,0)), smoothstep(0.,1.,f.x)), mix(rand(i + vec2(0.,1.)), rand(i + vec2(1.,1.)), smoothstep(0.,1.,f.x)), smoothstep(0.,1.,f.y));\n"
		"}\n";
	}
	if (funcs.contains("basic_lighting")) {
		source += "\nvec4 basic_lighting(vec3 normal, vec4 diffuse, float ambient, float specular, float shininess, vec4 emission) {\n"
		"	vec3 n = normalize(normal);\n"
		"	vec3 l = light.pos;\n"
		"	float d = max(-dot(n, l), 0);\n"
		"	vec4 r = ambient * light.color * (1 - light.harshness) / 2;\n"
		"	r += light.color * light.harshness * d;\n"
		"	r *= diffuse;\n"
		"	r += emission;\n"
		"	if ((d > 0) && (material.shininess > 1)) {\n"
		"		vec3 e = normalize(in_pos); // eye dir\n"
		"		vec3 rl = reflect(l, n);\n"
		"		float ee = max(-dot(e, rl), 0);\n"
		"		r += specular * light.color * light.harshness * pow(ee, shininess);\n"
		"	}\n"
		"	return r;\n"
		"}\n";
	}
	return source;
}

string ShaderGraph::build_fragment_source() const {
	string source = "#version 330 core\n"
			"#extension GL_ARB_separate_shader_objects : enable\n"
			"out vec4 out_color;\n";

	Set<string> dependencies;
	for (auto *n: nodes)
		for (string &f: n->dependencies)
			dependencies.add(f);

	Array<TempVar> temps;

	source += build_helper_vars(dependencies);
	source += build_helper_functions(dependencies);

	source += "\nvoid main() {\n";

	for (auto *n: sorted()) {
		if (n->type == "Output") {
			source += "\tout_color = " + sg_build_value(this, temps, n, 0) + ";\n";
		} else if (n->type == "Color") {
			string t = create_temp(temps, n, 0);
			source += "\tvec4 " + t + " = " + sg_build_constant(n->params[0]) + ";\n";
		} else if (n->type == "Vector") {
			string t = create_temp(temps, n, 0);
			source += "\tvec3 " + t + " = vec3(" + sg_build_value(this, temps, n, 0) + ", " + sg_build_value(this, temps, n, 1) + ", " + sg_build_value(this, temps, n, 2) + ");\n";
		} else if (n->type == "Texture") {
			string t = create_temp(temps, n, 0);
			source += "\tvec4 " + t + " = texture(tex0, " + sg_build_value(this, temps, n, 0, "in_uv") + ");\n";
		} else if (n->type == "Normals") {
			string t = create_temp(temps, n, 0);
			source += "\tvec3 " + t + " = normalize(in_normal);\n";
		} else if (n->type == "Position") {
			string t = create_temp(temps, n, 0);
			source += "\tvec3 " + t + " = in_pos;\n";
		} else if (n->type == "UV") {
			string t = create_temp(temps, n, 0);
			source += "\tvec2 " + t + " = in_uv;\n";
		} else if (n->type == "MultiplyColor") {
			string t = create_temp(temps, n, 0);
			source += "\tvec4 " + t + " = " + sg_build_value(this, temps, n, 0) + " * " + sg_build_value(this, temps, n, 1) + ";\n";
		} else if (n->type == "ColorRed") {
			string t = create_temp(temps, n, 0);
			source += "\tfloat " + t + " = " + sg_build_value(this, temps, n, 0) + ".r;\n";
		} else if (n->type == "RescaleVector") {
			string t = create_temp(temps, n, 0);
			source += "\tvec3 " + t + " = " + sg_build_value(this, temps, n, 0) + " * " + sg_build_value(this, temps, n, 1) + " + " + sg_build_value(this, temps, n, 2) + ";\n";
		} else if (n->type == "RescaleVector2") {
			string t = create_temp(temps, n, 0);
			source += "\tvec2 " + t + " = " + sg_build_value(this, temps, n, 0) + " * " + sg_build_value(this, temps, n, 1) + " + " + sg_build_value(this, temps, n, 2) + ";\n";
		} else if (n->type == "BasicLighting") {
			string t = create_temp(temps, n, 0);
			source += "\tvec4 " + t + " = basic_lighting(" + sg_build_value(this, temps, n, 5, "in_normal") + ", " + sg_build_value(this, temps, n, 0) + ", " + sg_build_value(this, temps, n, 1) + ", " + sg_build_value(this, temps, n, 2) + ", " + sg_build_value(this, temps, n, 3) + ", " + sg_build_value(this, temps, n, 4) + ");\n";
		} else if (n->type == "RandomFloat") {
			string t = create_temp(temps, n, 0);
			source += "\tfloat " + t + " = noise2d(in_uv);\n";
		} else if (n->type == "RandomColor") {
			string t = create_temp(temps, n, 0);
			source += "\tvec4 " + t + ";\n"
			"	" + t + ".r = noise2d(in_uv);\n"
			"	" + t + ".g = noise2d(in_uv);\n"
			"	" + t + ".b = noise2d(in_uv);\n"
			"	" + t + ".a = 1.0;\n";
		} else {
			source += " unhandled node " + n->type + "\n";

		}
	}
	source += "}\n";
	return source;
}

string ShaderGraph::build_source() const {
	string pre =
			"<VertexShader>\n"
			"#version 330 core\n"
			"#extension GL_ARB_separate_shader_objects : enable\n"
			"uniform mat4 mat_mvp;\n"
			"uniform mat4 mat_m;\n"
			"uniform mat4 mat_v;\n"
			"layout(location = 0) in vec3 in_position;\n"
			"layout(location = 1) in vec3 in_normal;\n"
			"layout(location = 2) in vec2 in_uv;\n"
			"layout(location = 0) out vec3 out_normal;\n"
			"layout(location = 1) out vec2 out_uv;\n"
			"layout(location = 2) out vec3 out_pos; // camera space\n"
			"void main() {\n"
			"	gl_Position = mat_mvp * vec4(in_position,1);\n"
			"	out_normal = (mat_v * mat_m * vec4(in_normal,0)).xyz;\n"
			"	out_uv = in_uv;\n"
			"	out_pos = (mat_v * mat_m * vec4(in_position,1)).xyz;\n"
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
	if (!can_cast(s->output[sp].type, d->params[dp].type))
		return;
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
