//
// Created by michi on 7/19/26.
//

#pragma once

#include "Instance.h"
#include <lib/os/path.h>
#include <functional>

namespace kaba {
	struct Class;
}

namespace plugin {

class PluginManager {
public:
	struct Plugin {
		PluginManager* plugin_manager;
		Path filename;
		string name;
		string category;

		void* create_instance(const string &parent) const;
	};
	Array<Plugin> plugins;
	Path directory;

	void* create_instance(const Path &filename, const string &parent);
	Array<const kaba::Class*> enumerate_classes(const string& full_base_class);
	//Array<plugin::InstanceData> enumerate_classes(const string& full_base_class);

	InstanceData describe_class(const kaba::Class* type, const void* instance = nullptr);
	void update_class(InstanceData& c);
	void set_variables(void* p, const kaba::Class* type, const Array<InstanceDataVariable>& variables);

	const kaba::Class* get_class(const InstanceData& desc) const;
};

Array<InstanceDataVariable> parse_variables_old(const string &var);

extern std::function<Any(const void*, const kaba::Class*)> f_whatever_to_any_special;
extern std::function<bool(void*, const kaba::Class*, const Any&)> f_whatever_from_any_special;

Any whatever_to_any(const void* p, const kaba::Class* c);
void whatever_from_any(void* p, const kaba::Class* type, const Any& value);

void assign_variables(void* p, const kaba::Class* c, const Array<InstanceDataVariable>& variables);

} // plugin
