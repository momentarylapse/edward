/*
 * BaseClass.cpp
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#include "BaseClass.h"
#include <lib/base/sort.h>


BaseClass::BaseClass(Type t) {
	type = t;
}

Any ScriptInstanceData::get(const string &name) const {
	for (const auto& v: variables)
		if (v.name == name)
			return v.value;
	return "";
}

void ScriptInstanceData::set(const string &name, const Any &value) {
	for (auto& v: variables)
		if (v.name == name) {
			v.value = value;
			return;
		}
	variables.add({name, value});
}

bool ScriptInstanceDataVariable::operator==(const ScriptInstanceDataVariable& other) const {
	return name == other.name and value == other.value;
}

bool ScriptInstanceDataVariable::operator!=(const ScriptInstanceDataVariable& other) const {
	return !(*this == other);
}


Array<ScriptInstanceDataVariable> sorted_variables(const Array<ScriptInstanceDataVariable>& variables) {
	return base::sorted(variables, [] (const ScriptInstanceDataVariable& a, const ScriptInstanceDataVariable& b) {
		return a.name < b.name;
	});
}

bool ScriptInstanceData::is_internal() const {
	return filename.is_empty() or filename.is_in("yengine");
}

bool ScriptInstanceData::operator==(const ScriptInstanceData& other) const {
	if (class_name != other.class_name)
		return false;
	if (filename != other.filename and !is_internal() and !other.is_internal())
		return false;
	auto vv = sorted_variables(variables);
	auto vv2 = sorted_variables(other.variables);
	return vv == vv2;
}

bool ScriptInstanceData::operator!=(const ScriptInstanceData& other) const {
	return !(*this == other);
}


