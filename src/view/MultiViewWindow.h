//
// Created by michi on 11.05.25.
//

#ifndef MULTIVIEWWINDOW_H
#define MULTIVIEWWINDOW_H

#include <y/renderer/Renderer.h>
#include <lib/math/mat4.h>
#include <lib/math/vec3.h>
#include <lib/math/quaternion.h>


struct RenderViewData;
class SceneRenderer;
class MultiView;



class MultiViewWindow {
public:
	explicit MultiViewWindow(MultiView* mv);
	vec3 project(const vec3& v) const;
	vec3 unproject(const vec3& v, const vec3& zref) const;
	vec3 direction() const;
	vec3 view_pos() const;
	quaternion view_ang() const;

	float zoom() const;
	float get_grid_d() const;
	int active_grid() const;
	vec3 active_grid_direction() const;
	mat3 active_grid_frame() const;
	mat3 edit_frame() const;
	void prepare(const RenderParams& params);
	void draw(const RenderParams& params);

	MultiView* multi_view;
	rect area;
	rect area_native;
	quaternion local_ang;
	mat4 to_pixels;
	mat4 view;
	mat4 projection;
	owned<SceneRenderer> scene_renderer;
	RenderViewData& rvd();
};


#endif //MULTIVIEWWINDOW_H
