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
	} else if (p.type == ShaderValueType::LITERAL) {
		return p.value;
	} else if (p.type == ShaderValueType::TEXT) {
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
	if (ts == ShaderValueType::VEC2 and td == ShaderValueType::COLOR)
		return "vec4(" + v + ", 0.0, 1.0)";
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
	if (ts == ShaderValueType::VEC2 and td == ShaderValueType::COLOR)
		return true;
	return false;
}

string ShaderBuilderContext::build_uniform_vars() {
	string source;
	for (auto &v: uniform_dependencies) {
		source += "uniform " + v + ";\n";
	}
	return source;
}

string ShaderBuilderContext::build_helper_vars() {
	string source;
	auto vars = dependencies;
#if 0
	if (vars.contains("in:light")) {
		source +=
R"foodelim(struct Light {
	mat4 proj;
	vec4 pos, dir, color;
	float radius, theta, harshness;
};
#ifdef vulkan
#else
/*layout(binding=1)*/ uniform LightData { Light light[32]; };
uniform int num_lights = 0;
#endif
)foodelim";
	}
	if (vars.contains("in:fog")) {
		source +=
			"struct Fog { vec4 color; float density; };\n"
			"uniform Fog fog;\n";
	}
	if (vars.contains("in:material")) {
		source +=
R"foodelim(struct Material {
	vec4 albedo, emission;
	float roughness, metal;
};
#ifdef vulkan
// push
#else
uniform Material material;
#endif
)foodelim";
	}
	if (vars.contains("in:pos")) {
		source += "layout(location=0) in vec4 in_pos; // view space\n";
	}
	if (vars.contains("in:normal")) {
		source += "layout(location=1) in vec3 in_normal;\n";
	}
	if (vars.contains("in:uv")) {
		source += "layout(location=2) in vec2 in_uv;\n";
	}
	if (vars.contains("in:color")) {
		source += "layout(location=3) in vec4 in_color;\n";
	}
	if (vars.contains("in:eye")) {
		source += "uniform vec3 eye_pos;\n";
	}
#endif
	if (vars.contains("texture0")) {
		source += "layout(binding=4) uniform sampler2D tex0;\n";
	}
	if (vars.contains("texture1")) {
		source += "layout(binding=5) uniform sampler2D tex1;\n";
	}
	if (vars.contains("texture2")) {
		source += "layout(binding=6) uniform sampler2D tex2;\n";
	}
	if (vars.contains("texture3")) {
		source += "layout(binding=7) uniform sampler2D tex3;\n";
	}
#if 0
	if (vars.contains("cubemap")) {
		source += "uniform samplerCube tex_cube;\n";
	}
	if (vars.contains("in:matrix")) {
		source +=
R"foodelim(struct Matrix {
	mat4 model, view, project;
};
#ifdef vulkan
layout(binding=0) uniform Matrix matrix;
#else
/*layout(binding=0)*/ uniform Matrix matrix;
#endif
)foodelim";
	}
	if (vars.contains("out:color")) {
		source += "layout(location=0) out vec4 out_color;\n";
	}
#endif


	return source;
}


string ShaderBuilderContext::build_helper_functions() {
	string source;
	auto funcs = dependencies;
	if (funcs.contains("rand2d")) {
		source += "\nfloat rand2d(vec2 st) {\n"
		"	return fract(sin(dot(st.xy ,vec2(12.9898,78.233))) * 43758.5453);\n"
		"}\n";
	}
	if (funcs.contains("rand3d")) {
		source += "\nfloat rand3d(vec3 p) {\n"
		"	return fract(sin(dot(p ,vec3(12.9898,78.233,4213.1234))) * 43758.5453);\n"
		"}\n";
	}
	if (funcs.contains("noise3d")) {
		source += "\nfloat noise3d(vec3 p) {\n"
		"	vec3 i = floor(p);\n"
		"	vec3 f = fract(p);\n"
		"	return mix(\n"
		"		mix(\n"
		"			mix(rand3d(i),               rand3d(i + vec3(1,0,0)), smoothstep(0,1,f.x)),\n"
		"			mix(rand3d(i + vec3(0,1,0)), rand3d(i + vec3(1,1,0)), smoothstep(0,1,f.x)),\n"
		"			smoothstep(0,1,f.y)),\n"
		"		mix(\n"
		"			mix(rand3d(i + vec3(0,0,1)), rand3d(i + vec3(1,0,1)), smoothstep(0,1,f.x)),\n"
		"			mix(rand3d(i + vec3(0,1,1)), rand3d(i + vec3(1,1,1)), smoothstep(0,1,f.x)),\n"
		"			smoothstep(0,1,f.y)),\n"
		"		smoothstep(0,1,f.z));\n"
		"}\n";
	}
	if (funcs.contains("noise3d_multi")) {
		source += "\nfloat noise3d_multi(vec3 p, float detail, float e) {\n"
		"	float r = 0;\n"
		"	float ff = fract(detail);\n"
		"	int i = 0;\n"
		"	while (i<detail) {\n"
		"		r += noise3d(p * pow(2, i)) * pow(e, i);\n"
		"		i ++;\n"
		"	}\n"
		"	r += ff * noise3d(p * pow(2, i)) * pow(e, i);\n"
		"	return r * (1-e) / (1- pow(e, detail));\n"
		"}\n";
	}
	if (funcs.contains("basic_lighting")) {
		source += "\nvec4 basic_lighting(vec3 n, vec4 albedo, float roughness, float metal, vec4 emission, Light l) {\n"
		"	vec3 L = (matrix.view * vec4(l.dir.xyz, 0)).xyz;\n"
		"	float d = max(-dot(n, L), 0);\n"
		"	vec4 r = roughness * l.color * (1 - l.harshness) / 2;\n"
		"	r += l.color * l.harshness * d;\n"
		"	r *= albedo;\n"
		"	r += emission;\n"
		"	if ((d > 0) && (material.roughness < 0.8)) {\n"
		"		vec3 e = normalize((matrix.view * in_pos).xyz); // eye dir\n"
		"		vec3 rl = reflect(L, n);\n"
		"		float ee = max(-dot(e, rl), 0);\n"
		"		r += (1 - roughness) * l.color * l.harshness * pow(ee, 5 / (1.1 - roughness));\n"
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

string ShaderBuilderContext::create_out(const ShaderNode *source, int port) {
	return create_temp(source, port, source->output[port].type);
}


string ShaderBuilderContext::find_temp(const ShaderNode *source, int port, ShaderValueType wanted_type) {
	for (auto &t: temps)
		if (t.source == source and t.port == port)
			return cast(t.name, t.type, wanted_type);
	return "???";
}

string ShaderBuilderContext::build_value(const ShaderNode *n, int i, const string &internal) {
	auto l = graph->find_source(n, i);
	if (l)
		return find_temp(l->source, l->source_port, n->params[i].type);
	if (internal != "")
		return internal;
	return sg_build_constant(n->params[i]);
}

string ShaderBuilderContext::build_const(const ShaderNode *n, int port) {
	return sg_build_constant(n->params[port]);
}



