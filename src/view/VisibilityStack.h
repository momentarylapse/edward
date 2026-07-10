#pragma once

#include <lib/base/pointer.h>
#include <lib/obs/Observable.h>
#include <view/Selection.h>

struct VisibilityFilter {
	bool bypass;
	const base::set<int>& allowed;
	bool operator()(int i) const;
};

class VisibilityStack : public obs::Node<VirtualBase> {
public:
	VisibilityStack();
	~VisibilityStack() override;
	void push(const Selection& selection);
	void pop();
	bool can_pop() const;

	VisibilityFilter get(MultiViewType type) const;

	Array<Selection> stack;
	base::set<int> dummy;
};

