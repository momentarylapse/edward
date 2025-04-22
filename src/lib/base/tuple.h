/*
 * tuple.h
 *
 *  Created on: Oct 16, 2020
 *      Author: michi
 */

#pragma once

namespace base {

template<class A, class B>
class tuple {
public:
	tuple() = default;
	tuple(const A& _a, const B& _b) {
		a = _a;
		b = _b;
	}
	void operator=(const tuple& o) {
		a = o.a;
		b = o.b;
	}
	A a;
	B b;
};

}



