/*
 * Component.cpp
 *
 *  Created on: Jul 12, 2021
 *      Author: michi
 */

#include "Component.h"
#include <lib/base/base.h>
#include <lib/config.h>
#ifdef _X_ALLOW_X_
#include "../plugins/PluginManager.h"
#endif

const kaba::Class* NameTag::_class = nullptr;

Component::Component() {
	owner = nullptr;
	component_type = nullptr;
}

Component::~Component() = default;

void Component::set_variables(const string &var) {
#ifdef _X_ALLOW_X_
	PluginManager::assign_variables(this, component_type, var);
#endif
}

