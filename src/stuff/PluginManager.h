/*
 * PluginManager.h
 *
 *  Created on: 01.07.2012
 *      Author: michi
 */

#ifndef PLUGINMANAGER_H_
#define PLUGINMANAGER_H_

#include <lib/base/base.h>
#include <lib/os/path.h>

class Session;

class PluginManager {
public:
	explicit PluginManager(Session* s, const Path &dir);
	~PluginManager();

	void init_edward();
	void link_plugins();
	void link_plugins_later();
	void find_plugins();

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
};

#endif /* PLUGINMANAGER_H_ */
