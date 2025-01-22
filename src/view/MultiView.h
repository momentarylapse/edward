//
// Created by Michael Ankele on 2025-01-20.
//

#ifndef MULTIVIEW_H
#define MULTIVIEW_H

#include <y/renderer/Renderer.h>
#include <y/renderer/world/geometry/SceneView.h>
#include <lib/math/vec3.h>
#include <lib/math/quaternion.h>

class Camera;
class Painter;
class Session;
class ActionMultiView;

class MultiView : public Renderer {
public:
	explicit MultiView(Session* session);
	~MultiView() override;

	void prepare(const RenderParams& params) override;

	void on_draw(Painter* p);
	void on_mouse_move(const vec2& m, const vec2& d);
	void on_mouse_leave();
	void on_mouse_wheel(const vec2& m, const vec2& d);
	void on_key_down(int key);

	struct ViewPort {
		vec3 pos;
		quaternion ang;
		float radius;
		Camera* cam;
		owned<SceneView> scene_view;

		void move(const vec3& drel);
		void rotate(const quaternion& qrel);
	} view_port;

	rect area;
	mat4 projection;
	Session* session;

	ActionMultiView* action = nullptr;
	mat4 action_trafo;
};



#endif //MULTIVIEW_H
