//
// Created by Michael Ankele on 2025-02-13.
//

#ifndef COMPONENTSELECTIONDIALOG_H
#define COMPONENTSELECTIONDIALOG_H

#include "../data/DataWorld.h"
#include <lib/base/future.h>
#include <lib/xhui/Dialog.h>

class Session;
struct ScriptInstanceData;

class ComponentSelectionDialog : public xhui::Dialog {
public:
	ComponentSelectionDialog(xhui::Panel* parent, Session* session, const string& base_class);
	base::promise<ScriptInstanceData> promise;
	Array<ScriptInstanceData> classes;

	static base::future<ScriptInstanceData> ask(xhui::Panel* parent, Session* session, const string& base_class = "ecs.Component");
};



#endif //COMPONENTSELECTIONDIALOG_H
