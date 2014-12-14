/*
 * PluginManager.h
 *
 *  Created on: 01.07.2012
 *      Author: michi
 */

#ifndef PLUGINMANAGER_H_
#define PLUGINMANAGER_H_

#include "../lib/file/file.h"

class PluginManager
{
public:
	PluginManager();
	virtual ~PluginManager();

	void init();
	void execute(const string &filename);
};

#endif /* PLUGINMANAGER_H_ */
