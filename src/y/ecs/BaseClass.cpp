/*
 * BaseClass.cpp
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#include "BaseClass.h"


BaseClass::BaseClass(Type t) {
	type = t;
}

string ScriptInstanceData::get(const string &name) const {
	for (const auto& v: variables)
		if (v.name == name)
			return v.value;
	return "";
}

void ScriptInstanceData::set(const string &name, const string &type, const string &value) {
	for (auto& v: variables)
		if (v.name == name) {
			v.value = value;
			v.type = type;
			return;
		}
	variables.add({name, type, value});
}


