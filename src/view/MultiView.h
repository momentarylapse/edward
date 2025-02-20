//
// Created by Michael Ankele on 2025-01-20.
//

#ifndef MULTIVIEW_H
#define MULTIVIEW_H

#include "Hover.h"
#include <y/renderer/Renderer.h>
#include <y/renderer/world/geometry/SceneView.h>
#include <y/renderer/world/geometry/RenderViewData.h>
#include <lib/math/Box.h>
#include <lib/math/vec3.h>
#include <lib/math/quaternion.h>
#include <lib/pattern/Observable.h>
#include <mode_model/data/SkinGenerator.h>
#include <functional>

class Camera;
class Painter;
class Session;
class ActionMultiView;
class ActionController;
class MultiView;
namespace multiview {
	struct SingleData;
}



class MultiViewWindow {
public:
	explicit MultiViewWindow(MultiView* mv);
	vec3 project(const vec3& v) const;
	vec3 unproject(const vec3& v, const vec3& zref) const;
	vec3 dir() const;
	float zoom() const;
	float get_grid_d() const;
	void draw(const RenderParams& params);

	MultiView* multi_view;
	rect area;
	rect area_native;
	quaternion local_ang;
	mat4 to_pixels;
	mat4 view;
	mat4 projection;
	RenderViewData rvd;
};

class MultiView : public obs::Node<Renderer> {
public:
	explicit MultiView(Session* session);
	~MultiView() override;

	obs::source out_selection_changed{this, "selection-changed"};
	obs::sink in_data_changed;

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	void on_draw(Painter* p);
	void on_left_button_down(const vec2& m);
	void on_left_button_up(const vec2& m);
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
		void suggest_for_box(const Box& box);
	} view_port;

	Array<Light*> lights;
	Light* default_light;

	MultiViewWindow window;
	MultiViewWindow* active_window;
	MultiViewWindow* hover_window;

	struct DataSet {
		MultiViewType type;
		DynamicArray* array;
		int flags;
	};
	Array<DataSet> data_sets;

	void clear_selection();
	void select_in_rect(MultiViewWindow* win, const rect& r);

	rect area;
	rect area_native;
	void set_area(const rect& area);
	Session* session;

	base::optional<Box> selection_box;
	//void set_selection_box(const base::optional<Box>& box);
	void update_selection_box();

	base::optional<rect> selection_area;
	bool possibly_selecting = false; // lbut down?

	ActionController* action_controller;

	std::function<base::optional<Hover>(MultiViewWindow*,const vec2&)> f_hover;
	base::optional<Hover> get_hover(MultiViewWindow* win, const vec2& m) const;
	base::optional<Hover> hover;

	multiview::SingleData* get_hover_item();
	vec3 cursor_pos_3d(const vec2& m) const;

	std::function<ActionMultiView*()> f_create_action;

	vec3 maybe_snap_v(const vec3& v) const;
	vec3 maybe_snap_v2(const vec3& v, float d) const;

	string get_unit_by_zoom(vec3 &v);
	string format_length(float l);
	void draw_mouse_pos(Painter* p);
};



#endif //MULTIVIEW_H
