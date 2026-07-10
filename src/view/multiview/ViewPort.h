//
// Created by michi on 6/15/26.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/optional.h>
#include <lib/base/pointer.h>
#include <lib/math/vec3.h>
#include <lib/math/quaternion.h>
#include <lib/obs/Observable.h>
#include <lib/yrenderer/scene/CameraParams.h>

struct Box;
class MultiView;

struct ViewPort : obs::Node<VirtualBase> {
	explicit ViewPort(MultiView* multi_view);
	vec3 pos;
	quaternion ang;
	float radius;
	yrenderer::CameraParams cam() const;
	MultiView* multi_view;

	void move(const vec3& drel);
	void rotate(const quaternion& qrel);
	void zoom(float factor, const base::optional<vec3>& focus_point);
	void suggest_for_box(const Box& box);
};
