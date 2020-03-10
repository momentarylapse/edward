/*
 * ShaderNode.cpp
 *
 *  Created on: Mar 10, 2020
 *      Author: michi
 */

#include "ShaderNode.h"
#include "ShaderBuilderContext.h"
#include "../../lib/image/color.h"

string sg_build_constant(ShaderNode::Parameter &p);

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



ShaderNode::ShaderNode(const string &_type, int _x, int _y) {
	type = _type;
	x = _x;
	y = _y;
}

class ShaderNodeColor : public ShaderNode {
public:
	ShaderNodeColor(const string &type, int x, int y) : ShaderNode(type, x, y) {
		params.add({ShaderValueType::COLOR, "value", "#ffffffff"});
		output.add({ShaderValueType::COLOR, "value"});
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tvec4 " + t + " = " + sg_build_constant(params[0]) + ";\n";
	}
};

class ShaderNodeVector : public ShaderNode {
public:
	ShaderNodeVector(const string &type, int x, int y) : ShaderNode(type, x, y) {
		params.add({ShaderValueType::FLOAT, "x", "0.0", "range=-1:1"});
		params.add({ShaderValueType::FLOAT, "y", "0.0", "range=-1:1"});
		params.add({ShaderValueType::FLOAT, "z", "1.0", "range=-1:1"});
		output.add({ShaderValueType::VEC3, "value"});
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tvec3 " + t + " = vec3(" + c->sg_build_value(this, 0) + ", " + c->sg_build_value(this, 1) + ", " + c->sg_build_value(this, 2) + ");\n";
	}
};

class ShaderNodeOutput : public ShaderNode {
public:
	ShaderNodeOutput(const string &type, int x, int y) : ShaderNode(type, x, y) {
		params.add({ShaderValueType::COLOR, "color", "#ff0000ff"});
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		return "\tout_color = " + c->sg_build_value(this, 0) + ";\n";
	}
};

class ShaderNodeTexture : public ShaderNode {
public:
	ShaderNodeTexture(const string &type, int x, int y) : ShaderNode(type, x, y) {
		params.add({ShaderValueType::VEC2, "uv", "-mesh-"});
		output.add({ShaderValueType::COLOR, "color"});
	}
	Array<string> dependencies() const override {
		return {"texture", "uv"};
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tvec4 " + t + " = texture(tex0, " + c->sg_build_value(this, 0, "in_uv") + ");\n";
	}
};

class ShaderNodeReflection : public ShaderNode {
public:
	ShaderNodeReflection(const string &type, int x, int y) : ShaderNode(type, x, y) {
		output.add({ShaderValueType::COLOR, "color"});
	}
	Array<string> dependencies() const override {
		return {"cubemap", "normal", "pos", "matview", "matworld"};
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tvec4 " + t + " = texture(tex4, (transpose(mat_v) * vec4(reflect(normalize((mat_v * mat_m * in_pos).xyz), normalize(in_normal)), 0)).xyz);\n";
	}
};

class ShaderNodeNormals : public ShaderNode {
public:
	ShaderNodeNormals(const string &type, int x, int y) : ShaderNode(type, x, y) {
		output.add({ShaderValueType::VEC3, "normals"});
	}
	Array<string> dependencies() const override {
		return {"normal"};
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tvec3 " + t + " = normalize(in_normal);\n";
	}
};

class ShaderNodeUV : public ShaderNode {
public:
	ShaderNodeUV(const string &type, int x, int y) : ShaderNode(type, x, y) {
		output.add({ShaderValueType::VEC2, "uv"});
	}
	Array<string> dependencies() const override {
		return {"uv"};
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tvec2 " + t + " = in_uv;\n";
	}
};

class ShaderNodePosition : public ShaderNode {
public:
	ShaderNodePosition(const string &type, int x, int y) : ShaderNode(type, x, y) {
		params.add({ShaderValueType::INT, "space", "0", "choice=model|world|camera|window"});
		output.add({ShaderValueType::VEC3, "pos"});
	}
	Array<string> dependencies() const override {
		return {"pos", "matproject", "matview", "matworld"};
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		if (params[0].value == "1") // world
			return "\tvec3 " + t + " = (mat_m * in_pos).xyz;\n";
		if (params[0].value == "2") // cam
			return "\tvec3 " + t + " = (mat_v * mat_m * in_pos).xyz;\n";
		if (params[0].value == "3") // window
			return "\tvec3 " + t + " = (mat_p * mat_v * mat_m * in_pos).xyz;\n";
		//if (params[0].value == "0") // model
		return "\tvec3 " + t + " = in_pos.xyz;\n";
	}
};

class ShaderNodeMultiply : public ShaderNode {
public:
	ShaderNodeMultiply(const string &type, int x, int y) : ShaderNode(type, x, y) {
		params.add({ShaderValueType::COLOR, "a", "#ffffffff"});
		params.add({ShaderValueType::COLOR, "b", "#ffffffff"});
		output.add({ShaderValueType::COLOR, "out"});
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tvec4 " + t + " = " + c->sg_build_value(this, 0) + " * " + c->sg_build_value(this, 1) + ";\n";
	}
};

class ShaderNodeMix : public ShaderNode {
public:
	ShaderNodeMix(const string &type, int x, int y) : ShaderNode(type, x, y) {
		params.add({ShaderValueType::COLOR, "a", "#ffffffff"});
		params.add({ShaderValueType::COLOR, "b", "#000000ff"});
		params.add({ShaderValueType::FLOAT, "factor", "0.5", "range=0:1"});
		output.add({ShaderValueType::COLOR, "out"});
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		string tt = c->create_temp(this, -1, ShaderValueType::FLOAT);
		return "\tfloat " + tt + " = " + c->sg_build_value(this, 2) + ";\n"
				"\tvec4 " + t + " = (1 - " + tt + ") * " + c->sg_build_value(this, 0) + " + " + tt + " * "+ c->sg_build_value(this, 1) + ";\n";
	}
};

class ShaderNodeAdd : public ShaderNode {
public:
	ShaderNodeAdd(const string &type, int x, int y) : ShaderNode(type, x, y) {
		params.add({ShaderValueType::COLOR, "a", "#000000ff"});
		params.add({ShaderValueType::COLOR, "b", "#000000ff"});
		output.add({ShaderValueType::COLOR, "out"});
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tvec4 " + t + " = " + c->sg_build_value(this, 0) + " + " + c->sg_build_value(this, 1) + ";\n";
	}
};

class ShaderNodeBrightness : public ShaderNode {
public:
	ShaderNodeBrightness(const string &type, int x, int y) : ShaderNode(type, x, y) {
		params.add({ShaderValueType::COLOR, "in", "#ffffffff"});
		output.add({ShaderValueType::FLOAT, "brightness"});
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string tt = c->create_temp(this, -1, ShaderValueType::COLOR);
		string t = c->create_temp(this, 0);
		return "\tvec4 " + tt + " = " + c->sg_build_value(this, 0) + ";\n"
				"\tfloat " + t + " = 0.2126 * " + tt + ".r + 0.7152 * " + tt + ".g + 0.0722 * " + tt + ".b;\n";
	}
};

class ShaderNodeVectorZ : public ShaderNode {
public:
	ShaderNodeVectorZ(const string &type, int x, int y) : ShaderNode(type, x, y) {
		params.add({ShaderValueType::VEC3, "in", "vec3(0,0,0)"});
		output.add({ShaderValueType::FLOAT, "z"});
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tfloat " + t + " = " + c->sg_build_value(this, 0) + ".z;\n";
	}
};


class ShaderNodeRandomColor : public ShaderNode {
public:
	ShaderNodeRandomColor(const string &type, int x, int y) : ShaderNode(type, x, y) {
		output.add({ShaderValueType::COLOR, "out"});
	}
	Array<string> dependencies() const override {
		return {"rand", "noise2d", "uv"};
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tvec4 " + t + ";\n"
		"	" + t + ".r = noise2d(in_uv);\n"
		"	" + t + ".g = noise2d(in_uv);\n"
		"	" + t + ".b = noise2d(in_uv);\n"
		"	" + t + ".a = 1.0;\n";
	}
};

class ShaderNodeRandomFloat : public ShaderNode {
public:
	ShaderNodeRandomFloat(const string &type, int x, int y) : ShaderNode(type, x, y) {
		output.add({ShaderValueType::FLOAT, "out"});
	}
	Array<string> dependencies() const override {
		return {"rand", "noise2d", "uv"};
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tfloat " + t + " = noise2d(in_uv);\n";
	}
};

class ShaderNodeRescaleVector : public ShaderNode {
public:
	ShaderNodeRescaleVector(const string &type, int x, int y) : ShaderNode(type, x, y) {
		params.add({ShaderValueType::VEC3, "in", "vec3(0,0,0)"});
		params.add({ShaderValueType::FLOAT, "scale", "1.0", "range=-2:2"});
		params.add({ShaderValueType::FLOAT, "offset", "0.0", "range=-10:10"});
		output.add({ShaderValueType::VEC3, "out"});
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tvec3 " + t + " = " + c->sg_build_value(this, 0) + " * " + c->sg_build_value(this, 1) + " + " + c->sg_build_value(this, 2) + ";\n";
	}
};

class ShaderNodeRescaleVector2 : public ShaderNode {
public:
	ShaderNodeRescaleVector2(const string &type, int x, int y) : ShaderNode(type, x, y) {
		params.add({ShaderValueType::VEC2, "in", "vec2(0,0)"});
		params.add({ShaderValueType::FLOAT, "scale", "1.0", "range=-2:2"});
		params.add({ShaderValueType::FLOAT, "offset", "0.0", "range=-10:10"});
		output.add({ShaderValueType::VEC2, "out"});
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tvec2 " + t + " = " + c->sg_build_value(this, 0) + " * " + c->sg_build_value(this, 1) + " + " + c->sg_build_value(this, 2) + ";\n";
	}
};

class ShaderNodeRescaleFloat : public ShaderNode {
public:
	ShaderNodeRescaleFloat(const string &type, int x, int y) : ShaderNode(type, x, y) {
		params.add({ShaderValueType::FLOAT, "in", "0.0"});
		params.add({ShaderValueType::FLOAT, "scale", "1.0", "range=-2:2"});
		params.add({ShaderValueType::FLOAT, "offset", "0.0", "range=-10:10"});
		output.add({ShaderValueType::FLOAT, "out"});
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tfloat " + t + " = " + c->sg_build_value(this, 0) + " * " + c->sg_build_value(this, 1) + " + " + c->sg_build_value(this, 2) + ";\n";
	}
};

class ShaderNodeBasicLighting : public ShaderNode {
public:
	ShaderNodeBasicLighting(const string &type, int x, int y) : ShaderNode(type, x, y) {
		params.add({ShaderValueType::COLOR, "diffuse", "#ffffffff"});
		params.add({ShaderValueType::FLOAT, "ambient", "0.3", "range=0:1"});
		params.add({ShaderValueType::FLOAT, "specular", "0.1", "range=0:1"});
		params.add({ShaderValueType::FLOAT, "shininess", "10.0", "range=0:50"});
		params.add({ShaderValueType::COLOR, "emission", "#000000ff"});
		params.add({ShaderValueType::VEC3, "normals", "-mesh-"});
		output.add({ShaderValueType::COLOR, "out"});
	}
	Array<string> dependencies() const override {
		return {"basic_lighting", "light", "matview", "matworld", "material", "pos", "normal"};
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tvec4 " + t + " = basic_lighting(" + c->sg_build_value(this, 5, "in_normal") + ", " + c->sg_build_value(this, 0) + ", " + c->sg_build_value(this, 1) + ", " + c->sg_build_value(this, 2) + ", " + c->sg_build_value(this, 3) + ", " + c->sg_build_value(this, 4) + ");\n";
	}
};

class ShaderNodeFog : public ShaderNode {
public:
	ShaderNodeFog(const string &type, int x, int y) : ShaderNode(type, x, y) {
		params.add({ShaderValueType::COLOR, "in", "#ffffffff"});
		output.add({ShaderValueType::COLOR, "out"});
	}
	Array<string> dependencies() const override {
		return {"fog", "pos", "matview", "matworld"};
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string tt = c->create_temp(this, -1, ShaderValueType::FLOAT);
		string t = c->create_temp(this, 0);
		return "\tfloat " + tt + " = exp(-(mat_v * mat_m * in_pos).z * fog.density);\n"
				"\tvec4 " + t + " = (1 - " + tt + ") * fog.color + " + tt + " * " + c->sg_build_value(this, 0) + ";\n";
	}
};

class ShaderNodeMaterialDiffuse : public ShaderNode {
public:
	ShaderNodeMaterialDiffuse(const string &type, int x, int y) : ShaderNode(type, x, y) {
		output.add({ShaderValueType::COLOR, "diffuse"});
	}
	Array<string> dependencies() const override {
		return {"material"};
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tvec4 " + t + " = " + c->sg_build_value(this, 5, "material.diffusive") + ";\n";
	}
};

class ShaderNodeMaterialSpecular : public ShaderNode {
public:
	ShaderNodeMaterialSpecular(const string &type, int x, int y) : ShaderNode(type, x, y) {
		output.add({ShaderValueType::FLOAT, "specular"});
	}
	Array<string> dependencies() const override {
		return {"material"};
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tvec4 " + t + " = " + c->sg_build_value(this, 5, "material.specular") + ";\n";
	}
};

class ShaderNodeMaterialEmission : public ShaderNode {
public:
	ShaderNodeMaterialEmission(const string &type, int x, int y) : ShaderNode(type, x, y) {
		output.add({ShaderValueType::COLOR, "emission"});
	}
	Array<string> dependencies() const override {
		return {"material"};
	}
	string code_pixel(ShaderBuilderContext *c) const override {
		string t = c->create_temp(this, 0);
		return "\tvec4 " + t + " = " + c->sg_build_value(this, 5, "material.emission") + ";\n";
	}
};

ShaderNode *create_node(const string &type, int x, int y) {
	if (type == "Color") {
		return new ShaderNodeColor(type, x, y);
	} else if (type == "Vector") {
		return new ShaderNodeVector(type, x, y);
	} else if (type == "Output") {
		return new ShaderNodeOutput(type, x, y);
	} else if (type == "Texture") {
		return new ShaderNodeTexture(type, x, y);
	} else if (type == "Reflection") {
		return new ShaderNodeReflection(type, x, y);
	} else if (type == "Normals") {
		return new ShaderNodeNormals(type, x, y);
	} else if (type == "UV") {
		return new ShaderNodeUV(type, x, y);
	} else if (type == "Position") {
		return new ShaderNodePosition(type, x, y);
	} else if (type == "Multiply") {
		return new ShaderNodeMultiply(type, x, y);
	} else if (type == "Mix") {
		return new ShaderNodeMix(type, x, y);
	} else if (type == "Add") {
		return new ShaderNodeAdd(type, x, y);
	} else if (type == "Brightness") {
		return new ShaderNodeBrightness(type, x, y);
	} else if (type == "VectorZ") {
		return new ShaderNodeVectorZ(type, x, y);
	} else if (type == "RandomColor") {
		return new ShaderNodeRandomColor(type, x, y);
	} else if (type == "RandomFloat") {
		return new ShaderNodeRandomFloat(type, x, y);
	} else if (type == "RescaleVector") {
		return new ShaderNodeRescaleVector(type, x, y);
	} else if (type == "RescaleVector2") {
		return new ShaderNodeRescaleVector2(type, x, y);
	} else if (type == "RescaleFloat") {
		return new ShaderNodeRescaleFloat(type, x, y);
	} else if (type == "BasicLighting") {
		return new ShaderNodeBasicLighting(type, x, y);
	} else if (type == "Fog") {
		return new ShaderNodeFog(type, x, y);
	} else if (type == "MaterialDiffuse") {
		return new ShaderNodeMaterialDiffuse(type, x, y);
	} else if (type == "MaterialSpecular") {
		return new ShaderNodeMaterialSpecular(type, x, y);
	} else if (type == "MaterialEmission") {
		return new ShaderNodeMaterialEmission(type, x, y);
	}
	throw Exception("unknown node type: " + type);
	return nullptr;
}
