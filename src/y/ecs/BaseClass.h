/*
 * BaseClass.h
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>

namespace ecs {

class BaseClass : public VirtualBase {
public:
	explicit BaseClass();
	virtual void _cdecl on_iterate(float dt) {}
	virtual void _cdecl on_init() {}
	virtual void _cdecl on_delete() {}
};

}
