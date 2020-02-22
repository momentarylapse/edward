/*
 * ShaderGraph.h
 *
 *  Created on: 21.02.2020
 *      Author: michi
 */

#ifndef SRC_DATA_MATERIAL_SHADERGRAPH_H_
#define SRC_DATA_MATERIAL_SHADERGRAPH_H_

#include "../../lib/base/base.h"

class color;


enum class ShaderValueType {
	FLOAT,
	INT,
	VEC2,
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
};

class ShaderGraph {
public:
	ShaderGraph();
	~ShaderGraph();

	void clear();

	string build_fragment_source() const;
	string build_source() const;

	Array<ShaderNode*> nodes;

	Array<ShaderNode*> sorted() const;

	struct Link {
		ShaderNode *source;
		int source_port;
		ShaderNode *dest;
		int dest_port;
	};
	Array<Link> links;

	ShaderNode *add(const string &type, int x, int y);
	void remove(ShaderNode *n);

	void connect(ShaderNode *s, int sp, ShaderNode *d, int dp);
	void unconnect(ShaderNode *s, int sp, ShaderNode *d, int dp);
	Link *find_source(ShaderNode *d, int dp) const;

	bool has_dependency(ShaderNode *s, ShaderNode *d) const;

	static const Array<string> NODE_TYPES;
};

#endif /* SRC_DATA_MATERIAL_SHADERGRAPH_H_ */
