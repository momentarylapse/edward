//
// Created by Michael Ankele on 2025-01-24.
//

#ifndef HOVER_H
#define HOVER_H

#include <lib/math/vec3.h>

enum class MultiViewType;

struct Hover {
	Hover() {}
	Hover(MultiViewType t, int i, const vec3& p) {
		type = t;
		index = i;
		tp = p;
	}
	MultiViewType type;
	int index;
	//int set;
	vec3 tp;
};

#endif //HOVER_H
