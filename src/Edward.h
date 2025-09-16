/*
 * Edward.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef EDWARD_H_
#define EDWARD_H_

#include <lib/xhui/Application.h>

class PluginManager;


class EdwardApp : public xhui::Application {
public:
	EdwardApp();
	//hui::AppStatus on_startup(const Array<string> &arg) override;
};

extern EdwardApp *app;

#endif /* EDWARD_H_ */
