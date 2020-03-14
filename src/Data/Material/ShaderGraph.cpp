/*
 * ShaderGraph.cpp
 *
 *  Created on: 21.02.2020
 *      Author: michi
 */

#include "ShaderGraph.h"
#include "ShaderNode.h"
#include "ShaderBuilderContext.h"
#include "../../lib/base/set.h"
#include "../../lib/image/color.h"
#include "../../lib/file/msg.h"
#include "../../lib/xfile/xml.h"
#include "../../lib/hui/hui.h"
#include "../../Stuff/PluginManager.h"
#include "../../Edward.h"


Array<string> ShaderGraph::enumerate() const {
	Array<string> r;
	Set<string> cats;
	for (auto &p: ed->plugins->plugins)
		if (p.type == PluginManager::PluginType::SHADER_NODE)
			cats.add(p.category);
	for (auto &c: cats) {
		r.add("--" + c);
		for (auto &p: ed->plugins->plugins)
			if (p.type == PluginManager::PluginType::SHADER_NODE)
				if (p.category == c)
					r.add(p.name);
	}
	return r;
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

	try {
		auto tex = add("Texture", -180, 50);
		auto mesh = add("Mesh", -180, 320);
		auto mat = add("Material", -180, 150);
		auto mul = add("Multiply", 50, 50);
		auto light = add("BasicLighting", 250, 100);
		auto out = add("Output", 480, 150);
		connect(mesh, 2, tex, 0);
		connect(tex, 0, mul, 0);
		connect(mat, 0, mul, 1);
		connect(mul, 0, light, 0);
		connect(mat, 1, light, 1);
		connect(mat, 2, light, 2);
		connect(mat, 3, light, 3);
		connect(mat, 4, light, 4);
		connect(mesh, 1, light, 5);
		connect(light, 0, out, 0);
	} catch (Exception &e) {
		ed->error_box(e.message());
	}
}

int ShaderGraph::node_index(const ShaderNode *n) const {
	foreachi (auto *nn, nodes, i)
		if (nn == n)
			return i;
	return -1;
}

// who needs sanity checks?!?!?
void ShaderGraph::load(const string &filename) {
	clear();

	xml::Parser p;
	p.load(filename);
	if (p.elements.num < 0)
		return;
	for (auto &e: p.elements[0].elements[0].elements) {
		string type = e.value("type");
		int x = e.value("x")._int();
		int y = e.value("y")._int();
		auto *n = create_node(type, x, y);
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
		connect(l.source, l.source_port, l.dest, l.dest_port);
	}
}

void ShaderGraph::save(const string &filename) {
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
		e.add_attribute("source", i2s(node_index(l.source)));
		e.add_attribute("sourceport", i2s(l.source_port));
		e.add_attribute("dest", i2s(node_index(l.dest)));
		e.add_attribute("destport", i2s(l.dest_port));
		elinks.add(e);
	}
	root.add(enodes);
	root.add(elinks);
	p.elements.add(root);
	p.save(filename);
}


bool ShaderGraph::has_dependency(const ShaderNode *s, const ShaderNode *d) const {
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
	int dep = level;
	for (auto *m: g->nodes)
		if (g->has_dependency(m, n))
			dep = max(dep, get_dep_depth(g, m, level + 1));
	return dep;
}

Array<ShaderNode*> ShaderGraph::sorted() const {
	Array<ShaderNode*> snodes;

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
				msg_error("failed to sort?!?");
			}
		}

	return snodes;
}

string ShaderGraph::build_fragment_source() const {
	string source = "#version 330 core\n"
			"#extension GL_ARB_separate_shader_objects : enable\n"
			"out vec4 out_color;\n";

	ShaderBuilderContext ctx(this);
	for (auto *n: nodes)
		for (string &f: n->dependencies())
			ctx.dependencies.add(f);

	source += ctx.build_helper_vars();
	source += ctx.build_helper_functions();

	source += "\nvoid main() {";
	for (auto *n: sorted()) {
		source += "\n\t// " + n->type + "";
		source += n->code_pixel(&ctx);
	}
	source += "\n}\n";
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
			"layout(location = 2) out vec4 out_pos; // model space\n"
			"void main() {\n"
			"	gl_Position = mat_mvp * vec4(in_position,1);\n"
			"	out_normal = (mat_v * mat_m * vec4(in_normal,0)).xyz;\n"
			"	out_uv = in_uv;\n"
			"	out_pos = vec4(in_position,1);\n"
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

ShaderGraph::Link *ShaderGraph::find_source(const ShaderNode *d, int dp) const {
	for (auto &l: links)
		if (l.dest == d and l.dest_port == dp)
			return &l;
	return nullptr;
}

ShaderNode* ShaderGraph::add(const string &type, int x, int y) {
	auto n = create_node(type, x, y);
	nodes.add(n);
	return n;
}

void ShaderGraph::connect(ShaderNode *s, int sp, ShaderNode *d, int dp) {
	if (sp < 0 or sp >= s->output.num)
		throw Exception("invalid source port: " + s->type + " #" + i2s(sp));
	if (dp < 0 or dp >= d->params.num)
		throw Exception("invalid target port: " + d->type + " #" + i2s(dp));
	if (!can_cast(s->output[sp].type, d->params[dp].type))
		throw Exception("invalid cast");
	//unconnect(s, sp, nullptr, -1);
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
