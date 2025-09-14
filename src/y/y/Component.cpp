/*
 * Component.cpp
 *
 *  Created on: Jul 12, 2021
 *      Author: michi
 */

#include "Component.h"
#include <lib/base/base.h>
#include <lib/config.h>
#include "../plugins/PluginManager.h"

const kaba::Class* NameTag::_class = nullptr;

Component::Component() {
	owner = nullptr;
	component_type = nullptr;
}

Component::~Component() = default;

void Component::set_variables(const Array<TemplateDataScriptVariable>& variables) {
	PluginManager::assign_variables(this, component_type, variables);
}

