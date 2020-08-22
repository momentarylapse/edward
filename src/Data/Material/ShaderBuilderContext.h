/*
 * ShaderBuilderContext.h
 *
 *  Created on: Mar 10, 2020
 *      Author: michi
 */

#ifndef SRC_DATA_MATERIAL_SHADERBUILDERCONTEXT_H_
#define SRC_DATA_MATERIAL_SHADERBUILDERCONTEXT_H_

#include "../../lib/base/base.h"
#include "../../lib/base/set.h"

enum class ShaderValueType;
class ShaderGraph;
class ShaderNode;


struct TempVar {
	string name;
	const ShaderNode *source;
	int port;
	ShaderValueType type;
};


bool can_cast(ShaderValueType ts, ShaderValueType td);

class ShaderBuilderContext {
public:
	ShaderBuilderContext(const ShaderGraph *g);

	Set<string> dependencies;
	Set<string> uniform_dependencies;
	Array<TempVar> temps;
	const ShaderGraph *graph;


	string create_temp(const ShaderNode *source, int port, ShaderValueType type);
	string create_out(const ShaderNode *source, int port);
	string find_temp(const ShaderNode *source, int port, ShaderValueType wanted_type);
	string build_value(const ShaderNode *n, int i, const string &internal = "");
	string build_const(const ShaderNode *n, int port);

	string build_uniform_vars();
	string build_helper_vars();
	string build_helper_functions();
};




#endif /* SRC_DATA_MATERIAL_SHADERBUILDERCONTEXT_H_ */
