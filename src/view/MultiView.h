//
// Created by Michael Ankele on 2025-01-20.
//

#ifndef MULTIVIEW_H
#define MULTIVIEW_H

#include <y/renderer/Renderer.h>
#include <y/renderer/world/geometry/SceneView.h>
#include <lib/math/vec3.h>
#include <lib/math/quaternion.h>
#include <lib/pattern/Observable.h>

class Camera;
class Painter;
class Session;
class ActionMultiView;
class ActionController;


enum {
	ACTION_SELECT,
	ACTION_MOVE,
	ACTION_ROTATE,
	ACTION_ROTATE_2D,
	ACTION_SCALE,
	ACTION_SCALE_2D,
	ACTION_MIRROR,
	ACTION_ONCE,
};

class MultiView : public obs::Node<Renderer> {
public:
	explicit MultiView(Session* session);
	~MultiView() override;

	obs::source out_selection_changed{this, "selection-changed"};

	void prepare(const RenderParams& params) override;

	void on_draw(Painter* p);
	void on_mouse_move(const vec2& m, const vec2& d);
	void on_mouse_leave();
	void on_mouse_wheel(const vec2& m, const vec2& d);
	void on_key_down(int key);

	struct ViewPort : obs::Node<VirtualBase> {
		explicit ViewPort(MultiView* multi_view);
		vec3 pos;
		quaternion ang;
		float radius;
		Camera* cam;
		owned<SceneView> scene_view;
		MultiView* multi_view;

		void move(const vec3& drel);
		void rotate(const quaternion& qrel);
		void zoom(float factor);
		void suggest_for_box(const vec3& vmin, const vec3& vmax);
	} view_port;

	rect area;
	mat4 projection;
	Session* session;

	ActionMultiView* action = nullptr;
	mat4 action_trafo;

	base::optional<rect> selection_area;

	ActionController* action_controller;
};



#endif //MULTIVIEW_H
