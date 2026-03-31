/*
 * Component.cpp
 *
 *  Created on: Jul 12, 2021
 *      Author: michi
 */

#include "Component.h"
#include <lib/base/base.h>
#include "../plugins/PluginManager.h"

const kaba::Class* NameTag::_class = nullptr;
const kaba::Class* EgoMarker::_class = nullptr;
const kaba::Class* TemplateRef::_class = nullptr;


namespace ecs {

Component::Component() {
	owner = nullptr;
	component_type = nullptr;
}

Component::~Component() = default;

void Component::set_variables(const Array<InstanceDataVariable>& variables) {
	PluginManager::assign_variables(this, component_type, variables);
}

}

