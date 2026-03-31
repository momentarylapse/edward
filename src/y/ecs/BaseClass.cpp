/*
 * BaseClass.cpp
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#include "BaseClass.h"
#include <lib/base/sort.h>


namespace ecs {

BaseClass::BaseClass(Type t) {
	type = t;
}

Any InstanceData::get(const string &name) const {
	for (const auto& v: variables)
		if (v.name == name)
			return v.value;
	return "";
}

void InstanceData::set(const string &name, const Any &value) {
	for (auto& v: variables)
		if (v.name == name) {
			v.value = value;
			return;
		}
	variables.add({name, value});
}

bool InstanceDataVariable::operator==(const InstanceDataVariable& other) const {
	return name == other.name and value == other.value;
}

bool InstanceDataVariable::operator!=(const InstanceDataVariable& other) const {
	return !(*this == other);
}


Array<InstanceDataVariable> sorted_variables(const Array<InstanceDataVariable>& variables) {
	return base::sorted(variables, [] (const InstanceDataVariable& a, const InstanceDataVariable& b) {
		return a.name < b.name;
	});
}

bool InstanceData::is_internal() const {
	return filename.is_empty() or filename.is_in("yengine");
}

bool InstanceData::operator==(const InstanceData& other) const {
	if (class_name != other.class_name)
		return false;
	if (filename != other.filename and !is_internal() and !other.is_internal())
		return false;
	auto vv = sorted_variables(variables);
	auto vv2 = sorted_variables(other.variables);
	return vv == vv2;
}

bool InstanceData::operator!=(const InstanceData& other) const {
	return !(*this == other);
}

}


