/*
 * ShaderGraph.h
 *
 *  Created on: 21.02.2020
 *      Author: michi
 */

#ifndef SRC_DATA_MATERIAL_SHADERGRAPH_H_
#define SRC_DATA_MATERIAL_SHADERGRAPH_H_

#include "../../lib/base/base.h"
#include "../../lib/base/pointer.h"

class Path;
class ShaderNode;

class ShaderGraph : public Sharable<base::Empty> {
public:
	ShaderGraph();
	~ShaderGraph();

	void clear();
	void make_default_for_engine();
	void make_default_basic();
	void make_default_cube_map();

	void load(const Path &filename);
	void save(const Path &filename);

	string build_fragment_source() const;
	string build_source() const;

	shared_array<ShaderNode> nodes;

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
	Link *find_source(const ShaderNode *d, int dp) const;

	bool has_dependency(const ShaderNode *s, const ShaderNode *d) const;

	int node_index(const ShaderNode *n) const;

	Array<string> enumerate() const;
};

#endif /* SRC_DATA_MATERIAL_SHADERGRAPH_H_ */
