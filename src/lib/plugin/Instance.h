
#pragma once

#include <lib/base/base.h>
#include <lib/os/path.h>
#include <lib/any/any.h>

namespace kaba {
	struct Class;
}

namespace plugin {

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
