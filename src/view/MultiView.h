//
// Created by Michael Ankele on 2025-01-20.
//

#ifndef MULTIVIEW_H
#define MULTIVIEW_H

#include "MultiViewWindow.h"
#include "Hover.h"
#include <lib/yrenderer/Renderer.h>
#include <lib/yrenderer/scene/SceneView.h>
#include <lib/yrenderer/scene/RenderViewData.h>
#include <lib/math/Box.h>
#include <lib/math/vec3.h>
#include <lib/math/quaternion.h>
#include <lib/pattern/Observable.h>
#include <functional>

#include "data/Data.h"

namespace yrenderer {
	class CubeMapSource;
	class CubeMapRenderer;
	class SceneRenderer;
	class ShadowRenderer;
}
class Painter;
class DocumentSession;
class Session;
class ActionMultiView;
class ActionController;
class MultiView;
namespace multiview {
	struct SingleData;
}



class MultiViewRenderer : public yrenderer::Renderer {
public:
	explicit MultiViewRenderer(yrenderer::Context* ctx, MultiView* mv);

	void prepare(const yrenderer::RenderParams& params) override;
	void draw(const yrenderer::RenderParams& params) override;

	MultiView* multi_view;
};

class MultiView : public obs::Node<VirtualBase> {
public:
	explicit MultiView(DocumentSession* doc);
	~MultiView() override;

	obs::source out_selection_changed{this, "selection-changed"};
	obs::sink in_data_changed;

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
		yrenderer::CameraParams cam;
		owned<yrenderer::SceneView> scene_view;
		MultiView* multi_view;

		void move(const vec3& drel);
		void rotate(const quaternion& qrel);
		void zoom(float factor, const base::optional<vec3>& focus_point);
		void suggest_for_box(const Box& box);
	} view_port;

	enum class LightMode {
		Fixed,
		FollowCamera
	} light_mode;

	Array<yrenderer::Light*> lights;
	yrenderer::Light* default_light;
	owned<yrenderer::ShadowRenderer> shadow_renderer;
	owned<yrenderer::CubeMapSource> cube_map_source;
	owned<yrenderer::CubeMapRenderer> cube_map_renderer;
	owned<MultiViewRenderer> renderer;

	owned<MultiViewWindow> window;
	MultiViewWindow* active_window;
	MultiViewWindow* hover_window;

	struct DataSet {
		MultiViewType type;
		DynamicArray* array;
		enum Flags {
			None = 0,
			Selectable = 1
		};
		Flags flags;
	};
	Array<DataSet> data_sets;

	void clear_selection();
	void invert_selection();
	void select_all();
	void select_in_rect(MultiViewWindow* win, const rect& r);
	static base::set<int> select_points_in_rect(MultiViewWindow* win, const rect& r, DynamicArray& array);

	rect area;
	rect area_native;
	void set_area(const rect& area);
	DocumentSession* doc;
	Session* session;
	yrenderer::Context* ctx;


	void set_allow_select(bool allow);
	void set_allow_action(bool allow);
	void set_show_grid(bool show);
	bool _allow_select = false;
	bool _show_grid = false;

	std::function<Data::Selection(MultiViewWindow*, const rect&)> f_select;
	std::function<void(Data::Selection&)> f_make_selection_consistent;
	std::function<base::optional<Box>(const Data::Selection&)> f_get_selection_box;
	base::optional<Box> selection_box;
	void update_selection_box();
	Data::Selection selection;
	Data::Selection temp_selection;

	static base::optional<Box> points_get_selection_box(const DynamicArray& array, const base::set<int>& sel);

	base::optional<rect> selection_area;
	bool possibly_selecting = false; // lbut down?

	owned<ActionController> action_controller;

	std::function<base::optional<Hover>(MultiViewWindow*,const vec2&)> f_hover;
	base::optional<Hover> get_hover(MultiViewWindow* win, const vec2& m) const;
	base::optional<Hover> hover;

	multiview::SingleData* get_hover_item();
	vec3 cursor_pos_3d(const vec2& m) const;
	base::optional<vec3> grid_hover_point(const vec2& m) const;

	std::function<ActionMultiView*()> f_create_action;

	static vec3 snap_v2(const vec3 &v, float d);
	vec3 snap_v(const vec3 &v) const;
	float snap_f(float f) const;
	vec3 maybe_snap_v(const vec3& v) const;
	vec3 maybe_snap_v2(const vec3& v, float d) const;
	float maybe_snap_f(float f) const;
	bool snap_to_grid = false;

	string get_unit_by_zoom(vec3 &v) const;
	string format_length(float l) const;
	void draw_mouse_pos(Painter* p);
};



#endif //MULTIVIEW_H
