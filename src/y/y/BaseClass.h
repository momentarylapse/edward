/*
 * BaseClass.h
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>
#include <lib/os/path.h>

namespace kaba {
	class Class;
}


class BaseClass : public VirtualBase {
public:
	enum class Type {
		NONE,
		ENTITY,
		SYSTEM,
		LINK,
		UI_NODE,
		UI_TEXT,
		UI_PICTURE,
		UI_MODEL,
	};

	explicit BaseClass(Type t);
	virtual void _cdecl on_iterate(float dt) {}
	virtual void _cdecl on_init() {}
	virtual void _cdecl on_delete() {}

	Type type;
};


struct ScriptInstanceDataVariable {
	string name, type, value;
};

struct ScriptInstanceData {
	string class_name;
	Path filename;
	Array<ScriptInstanceDataVariable> variables;
	string get(const string& name) const;
	void set(const string& name, const string& type, const string& value);
};
