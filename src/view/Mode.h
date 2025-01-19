//
// Created by Michael Ankele on 2025-01-20.
//

#ifndef MODE_H
#define MODE_H

#include "../lib/base/base.h"
#include "../lib/pattern/Observable.h"

struct SceneView;
class Renderer;
class MultiView;
class Session;

class Mode : obs::Node<VirtualBase> {
public:
	explicit Mode(Session* session);

	// this or a child
	virtual void enter_rec() {}
	virtual void leave_rec() {}

	// this exactly
	virtual void enter() {}
	virtual void leave() {}

	virtual Renderer* create_renderer(SceneView* scene_view) { return nullptr; }

	Session* session;
	MultiView* multi_view;
};



#endif //MODE_H
