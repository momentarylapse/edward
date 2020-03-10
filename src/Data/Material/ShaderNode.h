/*
 * ShaderNode.h
 *
 *  Created on: Mar 10, 2020
 *      Author: michi
 */

#ifndef SRC_DATA_MATERIAL_SHADERNODE_H_
#define SRC_DATA_MATERIAL_SHADERNODE_H_

#include "../../lib/base/base.h"

class color;
class ShaderBuilderContext;


enum class ShaderValueType {
	FLOAT,
	INT,
	VEC2,
	VEC3,
	COLOR
};

class ShaderNode {
public:
	ShaderNode(const string &type, int x, int y);
	virtual ~ShaderNode() {};

	int x, y;
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

	Array<string> dependencies;

	virtual string code_pixel(ShaderBuilderContext *ctx) const = 0;
};


string shader_value_type_to_str(ShaderValueType t);
ShaderNode *create_node(const string &type, int x, int y);


#endif /* SRC_DATA_MATERIAL_SHADERNODE_H_ */
