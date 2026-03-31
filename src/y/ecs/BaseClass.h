/*
 * BaseClass.h
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>
#include <lib/os/path.h>
#include <lib/any/any.h>

namespace kaba {
	class Class;
}

namespace ecs {

class BaseClass : public VirtualBase {
public:
	explicit BaseClass();
	virtual void _cdecl on_iterate(float dt) {}
	virtual void _cdecl on_init() {}
	virtual void _cdecl on_delete() {}
};


struct InstanceDataVariable {
	string name;
	Any value;
	bool operator==(const InstanceDataVariable& other) const;
	bool operator!=(const InstanceDataVariable& other) const;
};

struct InstanceData {
	string class_name;
	Path filename;
	Array<InstanceDataVariable> variables;
	Any get(const string& name) const;
	void set(const string& name, const Any& value);
	bool is_internal() const;
	bool operator==(const InstanceData& other) const;
	bool operator!=(const InstanceData& other) const;
};

}
