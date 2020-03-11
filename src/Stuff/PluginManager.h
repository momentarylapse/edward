/*
 * PluginManager.h
 *
 *  Created on: 01.07.2012
 *      Author: michi
 */

#ifndef PLUGINMANAGER_H_
#define PLUGINMANAGER_H_

#include "../lib/file/file.h"

class PluginManager {
public:
	PluginManager();
	virtual ~PluginManager();

	void init();
	void execute(const string &filename);

	static string directory;
	static void* create_instance(const string &filename, const string &parent);
};

#endif /* PLUGINMANAGER_H_ */
