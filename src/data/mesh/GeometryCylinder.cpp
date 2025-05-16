/*
 * GeometryCylinder.cpp
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#include "GeometryCylinder.h"
#include "GeometryBall.h"
#include <lib/math/mat4.h>
#include <lib/math/interpolation.h>

#define _cyl_vert(i, j)         ( edges      * (i) +(j) % edges)
#define _cyl_svert(i, j)        sv[(edges + 1) * (i) +(j) % (edges + 1)]

GeometryCylinder::GeometryCylinder(Array<vec3> &pos, Array<float> &radius, int rings, int edges, int end_mode)
{
	Interpolator<float> inter_r(Interpolator<float>::TYPE_CUBIC_SPLINE_NOTANG);
	for (float r: radius)
		inter_r.add(r);
	inter_r.normalize();

	// vertices (interpolated on path)
	Interpolator<vec3> inter(Interpolator<vec3>::TYPE_CUBIC_SPLINE_NOTANG);
	for (vec3 &p: pos)
		inter.add(p);
	inter.normalize();

	buildFromPath(inter, inter_r, rings, edges, end_mode);
}

GeometryCylinder::GeometryCylinder(Array<vec3> &pos, float radius, int rings, int edges, int end_mode)
{
	Interpolator<float> inter_r(Interpolator<float>::TYPE_CUBIC_SPLINE_NOTANG);
	inter_r.add(radius);
	inter_r.add(radius);
	inter_r.normalize();

	// vertices (interpolated on path)
	Interpolator<vec3> inter(Interpolator<vec3>::TYPE_CUBIC_SPLINE_NOTANG);
	for (vec3 &p: pos)
		inter.add(p);
	inter.normalize();

	buildFromPath(inter, inter_r, rings, edges, end_mode);
}

GeometryCylinder::GeometryCylinder(const vec3 &pos1, const vec3 &pos2, float radius, int rings, int edges, int end_mode)
{
	Interpolator<float> inter_r(Interpolator<float>::TYPE_CUBIC_SPLINE_NOTANG);
	inter_r.add(radius);
	inter_r.add(radius);
	inter_r.normalize();

	// vertices (interpolated on path)
	Interpolator<vec3> inter(Interpolator<vec3>::TYPE_CUBIC_SPLINE_NOTANG);
	inter.add(pos1);
	inter.add(pos2);
	inter.normalize();

	buildFromPath(inter, inter_r, rings, edges, end_mode);
}

void GeometryCylinder::__init2__(Array<vec3>& pos, Array<float>& radius, int rings, int edges, int end_mode)
{
	new (this) GeometryCylinder(pos, radius, rings, edges, end_mode);
}

void GeometryCylinder::__init__(const vec3& pos1, const vec3& pos2, float radius, int rings, int edges, int end_mode)
{
	new (this) GeometryCylinder(pos1, pos2, radius, rings, edges, end_mode);
}

mat4 make_frame(const vec3 &pos, const vec3 &dir, const vec3 &up, const vec3 right) {
	auto rot = mat4(right, up, dir);
	auto trans = mat4::translation(pos);
	return trans * rot;
}

static PolygonMesh half_ball(float radius, int edges, bool upper) {
	float scale = upper ? 1 : -1;
	auto ball = GeometryBall::create(v_0, radius, edges/2, edges);
	for (int i=ball.polygons.num-1; i>=0; i--) {
		vec3 m = v_0;
		for (int k=0; k<ball.polygons[i].side.num; k++)
			m += ball.vertices[ball.polygons[i].side[k].vertex].pos;
		if (m.y * scale > 0)
			ball.polygons.erase(i);
	}
	ball = ball.transform(mat4::rotation_x(pi/2));
	ball.remove_unused_vertices();
	return ball;
}

void GeometryCylinder::buildFromPath(Interpolator<vec3> &inter, Interpolator<float> &inter_r, int rings, int edges, int end_mode)
{
	if (end_mode == END_LOOP){
		inter.close();
		inter_r.close();
	}
	Array<vec3> sv;
	vec3 r_last = v_0;
	int rings_vertex = (end_mode == END_LOOP) ? rings : (rings + 1);
	mat4 frame0, frame1;
	for (int i=0; i<=rings; i++){
		// interpolated point on path
		float t = (float)i / (float)rings;
		vec3 p0 = inter.get(t);
		vec3 dir = inter.get_derivative(t).normalized();

		// moving frame
		vec3 u = vec3::cross(r_last, dir);
		if (i == 0)
			u = dir.ortho();
		u.normalize();
		vec3 r = vec3::cross(dir, u).normalized();
		r_last = r;
		mat4 frame = make_frame(p0, dir, u, r);
		if (i == 0)
			frame0 = frame;
		frame1 = frame;

		// vertex ring
		float radius = inter_r.get(t);
		for (int j=0; j<=edges; j++){
			float w = pi*2*(float)j/(float)edges;
			vec3 p = frame * (vec3((float)cos(w), (float)sin(w), 0) * radius);
			if ((j < edges) and (i < rings_vertex))
				add_vertex(p);
			sv.add(vec3((float)j / (float)edges, t, 0));
		}
	}

// the curved surface
	for (int i=0; i<rings_vertex-1; i++)
		for (int j=0; j<edges; j++){
			Array<int> v;
			v.add(_cyl_vert(i+1, j+1));
			v.add(_cyl_vert(i, j+1));
			v.add(_cyl_vert(i, j));
			v.add(_cyl_vert(i+1, j));
			Array<vec3> _sv;
			_sv.add(_cyl_svert(i+1, j+1));
			_sv.add(_cyl_svert(i, j+1));
			_sv.add(_cyl_svert(i, j));
			_sv.add(_cyl_svert(i+1, j));
			add_polygon_single_texture(v, _sv);
		}

	if (end_mode == END_LOOP){
		// how much did the 3-bein rotate?
		vec3 dir0 = inter.get_derivative(0);
		vec3 u0 = dir0.ortho();
		u0.normalize();
		vec3 r0 = vec3::cross(dir0, u0);
		r0.normalize();
		float phi = atan2(vec3::dot(u0, r_last), vec3::dot(r0, r_last));
		int dj = (int)((float)edges * (phi / 2 / pi));
		if (dj < edges)
			dj += (1-dj/edges) * edges;

		// close the last gap
		int i = rings - 1;
		for (int j=0; j<edges; j++){
			Array<int> v;
			v.add(_cyl_vert(0, j+1+dj));
			v.add(_cyl_vert(i, j+1));
			v.add(_cyl_vert(i, j));
			v.add(_cyl_vert(0, j+dj));
			Array<vec3> _sv;
			_sv.add(_cyl_svert(rings, j+1));
			_sv.add(_cyl_svert(i, j+1));
			_sv.add(_cyl_svert(i, j));
			_sv.add(_cyl_svert(rings, j));
			add_polygon_single_texture(v, _sv);
		}
	}

// the endings

	if (end_mode == END_FLAT){
		// skin vertices
		sv.clear();
		for (int j=0; j<edges; j++){
			float w = pi*2 * (float)j / (float)edges;
			sv.add(vec3(0.5f + (float)sin(w)/2, 0.5f + (float)cos(w)/2, 0));
		}

		// polygons
		Array<int> v;
		for (int j=0;j<edges;j++)
			v.add(j);
		add_polygon_single_texture(v, sv);
		v.clear();
		for (int j=0;j<edges;j++)
			v.add(vertices.num - j - 1);
		add_polygon_single_texture(v, sv);
	}

	if (end_mode == END_ROUND){

		float r0 = inter_r.get(0);
		float r1 = inter_r.get(1);

		auto b0 = half_ball(r0, edges, true);
		add(b0.transform(frame0).invert());
		auto b1 = half_ball(r1, edges, false);
		add(b1.transform(frame1).invert());

		weld(r0 * 0.001f);
	}
}

