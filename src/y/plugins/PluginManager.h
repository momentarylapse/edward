/*
 * PluginManager.h
 *
 *  Created on: 02.01.2020
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>

class Path;
class Profiler;
struct ScriptInstanceDataVariable;
namespace kaba {
	class Class;
	class Exporter;
}
struct vec3;
struct color;

namespace PluginManager {
	void init();
	void export_kaba_package_y(kaba::Exporter* exporter);
	void import_kaba();

	const kaba::Class *find_class(const Path &filename, const string &name);
	const kaba::Class *find_class_derived(const Path &filename, const string &base_class);
	void *create_instance(const kaba::Class *type, const string &variables);
	void *create_instance(const kaba::Class *type, const Array<ScriptInstanceDataVariable> &variables);
	void *create_instance_auto(const string& extended_type_name);
	void assign_variables(void *p, const kaba::Class *c, const Array<ScriptInstanceDataVariable> &variables);

	string whatever_to_string(const void* instance, int offset, const kaba::Class* c);
	void whatever_from_string(void* p, const kaba::Class* type, const string& value);

	Array<ScriptInstanceDataVariable> parse_variables(const string &var);

	vec3 s2v(const string &s);
	// RGBA
	color s2c(const string &s);

};

