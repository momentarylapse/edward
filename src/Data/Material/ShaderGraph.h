/*
 * ShaderGraph.h
 *
 *  Created on: 21.02.2020
 *      Author: michi
 */

#ifndef SRC_DATA_MATERIAL_SHADERGRAPH_H_
#define SRC_DATA_MATERIAL_SHADERGRAPH_H_

#include "../../lib/base/base.h"


enum class ShaderValueType {
	FLOAT,
	INT,
	VEC3,
	COLOR
};

string shader_value_type_to_str(ShaderValueType t);


class ShaderNode {
public:
	ShaderNode(const string &type, int x, int y);

	int x, y;
	string type;

	struct Parameter {
		ShaderValueType type;
		string name;
		string value;
	};
	Array<Parameter> params;


	struct Port {
		ShaderValueType type;
		string name;
	};
	Array<Port> output;
};

class ShaderGraph {
public:
	ShaderGraph();
	~ShaderGraph();

	string build() const;

	Array<ShaderNode*> nodes;

	struct Link {
		ShaderNode *source;
		int source_port;
		ShaderNode *dest;
		int dest_port;
	};
	Array<Link> links;

	ShaderNode *add(const string &type, int x, int y);
	void connect(ShaderNode *s, int sp, ShaderNode *d, int dp);

	Link *find_source(ShaderNode *d, int dp) const;
};

#endif /* SRC_DATA_MATERIAL_SHADERGRAPH_H_ */
