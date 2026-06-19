#include "VisibilityStack.h"

bool VisibilityFilter::operator()(int i) const {
	if (bypass)
		return true;
	return allowed.contains(i);
}

VisibilityStack::VisibilityStack() = default;
VisibilityStack::~VisibilityStack() = default;

void VisibilityStack::push(const Selection& selection) {
	stack.add(selection);
	out_changed();
}

void VisibilityStack::pop() {
	stack.pop();
	out_changed();
}

bool VisibilityStack::can_pop() const {
	return stack.num > 0;
}

VisibilityFilter VisibilityStack::get(MultiViewType type) const {
	if (stack.num > 0 and stack.back().contains(type))
		return {false, stack.back()[type]};
	return {true, dummy};
}
