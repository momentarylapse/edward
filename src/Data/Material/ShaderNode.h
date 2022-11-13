/*
 * ShaderNode.h
 *
 *  Created on: Mar 10, 2020
 *      Author: michi
 */

#ifndef SRC_DATA_MATERIAL_SHADERNODE_H_
#define SRC_DATA_MATERIAL_SHADERNODE_H_

#include "../../lib/base/base.h"
#include "../../lib/base/pointer.h"
#include "../../lib/math/vec2.h"

class color;
class ShaderBuilderContext;


enum class ShaderValueType {
	FLOAT,
	INT,
	VEC2,
	VEC3,
	COLOR,
	LITERAL,
	TEXT
};

class ShaderNode : public Sharable<VirtualBase> {
public:
	ShaderNode(const string &type);
	virtual ~ShaderNode() {};

	void __init__(const string &t);
	virtual void __delete__();

	vec2 pos;
	string type;

	struct Parameter {
		ShaderValueType type;
		string name;
		string value;
		string options;
		color get_color() const;
		void set_color(const color &c);
	};
	Array<Parameter> params;


	struct Port {
		ShaderValueType type;
		string name;
	};
	Array<Port> output;

	virtual Array<string> dependencies() const { return {}; };

	virtual Array<string> uniform_dependencies() const { return {}; };

	virtual string code_pixel(ShaderBuilderContext *ctx) const { return "?"; };
	virtual string code_pixel_pre(ShaderBuilderContext *ctx) const { return ""; };
};


string shader_value_type_to_str(ShaderValueType t);
ShaderNode *create_node(const string &type, int x, int y);


#endif /* SRC_DATA_MATERIAL_SHADERNODE_H_ */
