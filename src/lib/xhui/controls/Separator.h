//
// Created by Michael Ankele on 2025-04-16.
//

#ifndef SEPARATOR_H
#define SEPARATOR_H

#include "Control.h"

namespace xhui {

class Separator : public Control {
public:
	explicit Separator(const string& id);
	vec2 get_content_min_size() const override;
	vec2 get_greed_factor() const override;
	void _draw(Painter* p) override;
};

} // xhui

#endif //SEPARATOR_H
