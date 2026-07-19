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
namespace plugin {
	struct InstanceDataVariable;
}
namespace kaba {
	struct Class;
	class IExporter;
}
class ResourceManager;
struct vec3;
struct color;

namespace PluginManager {
	void init();
	void init_basic();
	void export_kaba_package_yengine(kaba::IExporter* exporter);
	void import_kaba();

	const kaba::Class* find_class(const Path &filename, const string &name);
	const kaba::Class* find_class_derived(const Path &filename, const string &base_class);
	void* create_instance(const kaba::Class *type, const string &variables);
	void* create_instance(const kaba::Class *type, const Array<plugin::InstanceDataVariable> &variables);
	void* create_instance_auto(const string& extended_type_name);

	extern ResourceManager* default_resource_manager;

	vec3 s2v(const string &s);
	// RGBA
	color s2c(const string& s);

};

