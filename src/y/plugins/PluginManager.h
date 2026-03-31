/*
 * PluginManager.h
 *
 *  Created on: 02.01.2020
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>

class Any;
class Path;
class Profiler;
namespace ecs {
	struct InstanceDataVariable;
}
namespace kaba {
	class Class;
	class Exporter;
}
class ResourceManager;
struct vec3;
struct color;

namespace PluginManager {
	void init();
	void export_kaba_package_yengine(kaba::Exporter* exporter);
	void import_kaba();

	const kaba::Class* find_class(const Path &filename, const string &name);
	const kaba::Class* find_class_derived(const Path &filename, const string &base_class);
	void* create_instance(const kaba::Class *type, const string &variables);
	void* create_instance(const kaba::Class *type, const Array<ecs::InstanceDataVariable> &variables);
	void* create_instance_auto(const string& extended_type_name);
	void assign_variables(void *p, const kaba::Class *c, const Array<ecs::InstanceDataVariable> &variables);

	Any whatever_to_any(const void* p, const kaba::Class* c);
	void whatever_from_any(void* p, const kaba::Class* type, const Any& value);

	Array<ecs::InstanceDataVariable> parse_variables(const string &var);
	extern ResourceManager* default_resource_manager;

	vec3 s2v(const string &s);
	// RGBA
	color s2c(const string& s);

};

