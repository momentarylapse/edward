/*
 * ShaderBuilderContext.cpp
 *
 *  Created on: Mar 10, 2020
 *      Author: michi
 */

#include "ShaderBuilderContext.h"
#include "ShaderGraph.h"
#include "ShaderNode.h"
#include "../../lib/image/color.h"


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



string cast(const string &v, ShaderValueType ts, ShaderValueType td) {
	if (ts == ShaderValueType::FLOAT and td == ShaderValueType::VEC3)
		return "(vec3(1,1,1) * " + v + ")";
	if (ts == ShaderValueType::FLOAT and td == ShaderValueType::COLOR)
		return "vec4(vec3(1,1,1) * " + v + ", 1)";
	if (ts == ShaderValueType::COLOR and td == ShaderValueType::VEC3)
		return v + ".xyz";
	if (ts == ShaderValueType::VEC3 and td == ShaderValueType::COLOR)
		return "vec4(" + v + ", 1.0)";
	return v;
}

bool can_cast(ShaderValueType ts, ShaderValueType td) {
	if (ts == td)
		return true;
	if (ts == ShaderValueType::FLOAT and td == ShaderValueType::VEC3)
		return true;
	if (ts == ShaderValueType::FLOAT and td == ShaderValueType::COLOR)
		return true;
	if (ts == ShaderValueType::COLOR and td == ShaderValueType::VEC3)
		return true;
	if (ts == ShaderValueType::VEC3 and td == ShaderValueType::COLOR)
		return true;
	return false;
}

string ShaderBuilderContext::build_helper_vars() {
	string source;
	auto vars = dependencies;
	if (vars.contains("light")) {
		source +=
			"struct Light { vec4 color; vec4 pos; float radius, harshness; };\n"
			"uniform LightBlock { Light light; };\n";
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
		source += "layout(location = 0) in vec3 in_normal;\n";
	}
	if (vars.contains("uv")) {
		source += "layout(location = 1) in vec2 in_uv;\n";
	}
	if (vars.contains("pos")) {
		source += "layout(location = 2) in vec3 in_pos;\n";
	}
	if (vars.contains("texture")) {
		source += "uniform sampler2D tex0;\n";
	}
	if (vars.contains("cubemap")) {
		source += "uniform samplerCube tex4;\n";
	}
	if (vars.contains("matview")) {
		source += "uniform mat4 mat_v;\n";
	}
	if (vars.contains("matworld")) {
		source += "uniform mat4 mat_m;\n";
	}
	if (vars.contains("matproject")) {
		source += "uniform mat4 mat_p;\n";
	}

	return source;
}


string ShaderBuilderContext::build_helper_functions() {
	string source;
	auto funcs = dependencies;
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
		source += "\nvec4 basic_lighting(vec3 n, vec4 diffuse, float ambient, float specular, float shininess, vec4 emission) {\n"
		"	vec3 l = (mat_v * vec4(light.pos.xyz, 0)).xyz;\n"
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


ShaderBuilderContext::ShaderBuilderContext(const ShaderGraph *g) {
	graph = g;
}
string ShaderBuilderContext::create_temp(const ShaderNode *source, int port, ShaderValueType type) {
	TempVar t;
	t.name = format("tmp%d", temps.num + 1);
	t.source = source;
	t.port = port;
	t.type = type;
	temps.add(t);
	return t.name;
}

string ShaderBuilderContext::create_temp(const ShaderNode *source, int port) {
	return create_temp(source, port, source->output[port].type);
}


string ShaderBuilderContext::find_temp(const ShaderNode *source, int port, ShaderValueType wanted_type) {
	for (auto &t: temps)
		if (t.source == source and t.port == port)
			return cast(t.name, t.type, wanted_type);
	return "???";
}

string ShaderBuilderContext::sg_build_value(const ShaderNode *n, int i, const string &internal) {
	auto l = graph->find_source(n, i);
	if (l)
		return find_temp(l->source, l->source_port, n->params[i].type);
	if (internal != "")
		return internal;
	return sg_build_constant(n->params[i]);
}



