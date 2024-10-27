/*
 * ShaderNode.cpp
 *
 *  Created on: Mar 10, 2020
 *      Author: michi
 */

#include "ShaderNode.h"
#include "ShaderBuilderContext.h"
#include "../../lib/image/color.h"
#include "../../lib/os/msg.h"
#include "../../stuff/PluginManager.h"
#include "../../EdwardWindow.h"
#include "../../Edward.h"

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
	if (t == ShaderValueType::LITERAL)
		return "literal";
	if (t == ShaderValueType::TEXT)
		return "text";
	return "???";
}



color ShaderNode::Parameter::get_color() const {
	if (type != ShaderValueType::COLOR)
		return Black;
	return color::parse(value);
}

void ShaderNode::Parameter::set_color(const color &c) {
	if (type != ShaderValueType::COLOR)
		return;
	value = c.hex();
}


ShaderNode::ShaderNode(const string &_type) {
	type = _type;
	pos = {0,0};
}

void ShaderNode::__init__(const string &t) {
	new(this) ShaderNode(t);
}

void ShaderNode::__delete__() {
	this->~ShaderNode();
}


ShaderNode *create_node(const string &type, int x, int y) {
	try {
		for (auto &p: app->plugins->plugins)
			if (p.type == PluginManager::PluginType::SHADER_NODE)
				if (p.name == type) {
					auto n = (ShaderNode*)p.create_instance("*.shader.Node");
					n->pos = vec2(x,y);
					return n;
		}
		throw Exception("node type unknown: " + type);
	} catch (Exception &e) {
		msg_error(e.message());
	}
	return nullptr;
}
