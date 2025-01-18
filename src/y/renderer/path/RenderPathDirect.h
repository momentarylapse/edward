//
// Created by michi on 1/3/25.
//

#ifndef RENDERPATHDIRECT_H
#define RENDERPATHDIRECT_H

#include "RenderPath.h"

class RenderPathDirect : public RenderPath {
public:
	explicit RenderPathDirect(Camera* cam);
	void prepare(const RenderParams &params) override;
	void draw(const RenderParams &params) override;
};


#endif //RENDERPATHDIRECT_H
