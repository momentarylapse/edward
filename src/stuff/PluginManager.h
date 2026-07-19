/*
 * PluginManager.h
 *
 *  Created on: 01.07.2012
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>
#include <lib/os/path.h>
#include <lib/plugin/Instance.h>

namespace kaba {
	struct Class;
}

class Session;

namespace edward {

class PluginManager {
public:
	explicit PluginManager(Session* s, const Path &dir);
	~PluginManager();

	void init_edward();
	void link_plugins();
	void link_plugins_later();
	void find_plugins();

	void load_project_stuff(const Path& dir);

	enum class PluginType {
		PLUGIN,
		SHADER_NODE,
	};

	struct Plugin {
		PluginManager* plugin_manager;
		Path filename;
		string name;
		string category;
		PluginType type;

		void* create_instance(const string &parent) const;
	};
	Array<Plugin> plugins;
	Session* session;
	Path directory;

	void execute(const Path &filename);

	void* create_instance(const Path &filename, const string &parent);
	Array<const kaba::Class*> enumerate_classes(const string& full_base_class);
	//Array<plugin::InstanceData> enumerate_classes(const string& full_base_class);

	plugin::InstanceData describe_class(const kaba::Class* type, const void* instance = nullptr);
	void update_class(plugin::InstanceData& c);

	Array<const kaba::Class*> component_classes;
	Array<const kaba::Class*> system_classes;

	const kaba::Class* get_class(const plugin::InstanceData& desc) const;
};

}
