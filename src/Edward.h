/*
 * Edward.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef EDWARD_H_
#define EDWARD_H_

#include "lib/hui/hui.h"

class PluginManager;


class EdwardApp : public hui::Application {
public:
	EdwardApp();
	hui::AppStatus on_startup(const Array<string> &arg) override;


	PluginManager *plugins;
};

extern EdwardApp *app;

#endif /* EDWARD_H_ */
