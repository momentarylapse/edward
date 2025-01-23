/*
 * DataCamera.cpp
 *
 *  Created on: 30.09.2012
 *      Author: michi
 */

#include "DataCamera.h"
#include "../../Session.h"
#include "../../storage/Storage.h"
#include <lib/os/file.h>
#include <lib/os/formatter.h>
#include <lib/os/msg.h>

DataCamera::DataCamera(Session *s) :
	Data(s, FD_CAMERAFLIGHT)
{
}

DataCamera::~DataCamera() {
}

void DataCamera::reset() {
	filename = "";
	Point.clear();
	Vel.clear();
	reset_history();
	out_changed();
}

bool DataCamera::load(const Path &_filename, bool deep) {
#if 0
	bool Error = false;
	reset();

	filename = _filename;
	os::fs::FileStream *f = nullptr;

	try{

	f = os::fs::open(filename, "rt");
	f->read(1);
	int ffv=f->read_word();
	if (ffv == 2) {

		f->read_comment();
		int n = f->read_int();
		//msg_write(NumCamPoints);
		for (int i=0;i<n;i++){
			WorldCamPoint c;
			memset(&c, 0, sizeof(WorldCamPoint));
			f->read_comment();
			c.Type=f->read_int();
			if (c.Type == CPKSetCamPos){
				f->read_vector(&c.pos);
				c.Duration = f->read_float();
			}else if (c.Type == CPKSetCamPosRel){
				f->read_vector(&c.pos);
				c.Duration = f->read_float();
			}else if (c.Type == CPKSetCamAng){
				f->read_vector(&c.Ang);
				c.Duration = f->read_float();
			}else if (c.Type == CPKSetCamPosAng){
				f->read_vector(&c.pos);
				f->read_vector(&c.Ang);
				c.Duration = f->read_float();
			}else if (c.Type == CPKCamFlight){
				f->read_vector(&c.pos);
				f->read_vector(&c.Vel);
				f->read_vector(&c.Ang);
				c.Duration = f->read_float();
			}
			if ((c.Type == CPKSetCamPosAng) && (c.Duration > 0)){
				WorldCamPoint c2 = c;
				c2.Duration = 0;
				Point.add(c2);
				c.Type = CPKCamFlight;
			}
			Point.add(c);
		}

	}else{
		session->error(format("Invalid file format of the file %s: %d (%d expected)!", filename, ffv, 2));
		Error=true;
	}
	delete f;
	UpdateVel();
	reset_history();
	out_changed();

	}catch(Exception &e){
		session->set_message(e.message());
	}

	return !Error;
#endif
	return false;
}

bool DataCamera::save(const Path &_filename)
{
	filename = _filename;
	os::fs::FileStream *f = nullptr;

	try{

	f = os::fs::open(filename, "wt");
	//f->float_decimals = 4;
	f->write("t");
	f->write_word(2);

	f->write_comment("// Number Of CamPoints");
	f->write_int(Point.num);
	foreachi(WorldCamPoint &c, Point, i){
		f->write_comment(format("// Point No.%d", i));
		f->write_int(c.Type);
		if (c.Type == CPKSetCamPos){
			f->write_vector(&c.pos);
			f->write_float(c.Duration);
		}else if (c.Type == CPKSetCamPosRel){
			f->write_vector(&c.pos);
			f->write_float(c.Duration);
		}else if (c.Type == CPKSetCamAng){
			f->write_vector(&c.Ang);
			f->write_float(c.Duration);
		}else if (c.Type == CPKSetCamPosAng){
			f->write_vector(&c.pos);
			f->write_vector(&c.Ang);
			f->write_float(c.Duration);
		}else if (c.Type == CPKCamFlight){
			f->write_vector(&c.pos);
			f->write_vector(&c.Vel);
			f->write_vector(&c.Ang);
			f->write_float(c.Duration);
		}
	}
	f->write_comment("#");

	delete f;
	session->set_message("Camera script saved!");
	action_manager->mark_current_as_save();

	} catch(Exception &e) {
		msg_error(e.message());
	}

	return true;
}


void DataCamera::UpdateVel()
{
	Vel.resize(Point.num);
	foreachi(WorldCamPoint &c, Point, i){
		Vel[i].is_special = true;
		Vel[i].pos = c.pos + c.Vel;
	}
}

float DataCamera::GetDuration() const
{
	float d = 0;
	for (int i=0; i<Point.num; i++)
		d += Point[i].Duration;
	return d;
}

Interpolator<vec3> DataCamera::BuildPosInterpolator() const
{
	Interpolator<vec3> inter(Interpolator<vec3>::TYPE_CUBIC_SPLINE);
	if (Point.num > 0)
		if (Point[0].Type == CPKCamFlight)
			inter.jump(v_0, v_0);
	for (int i=0; i<Point.num; i++){
		if (Point[i].Type == CPKCamFlight){
			inter.add2(Point[i].pos, Point[i].Vel, Point[i].Duration);
		}else if (Point[i].Type == CPKSetCamPosAng){
			inter.jump(Point[i].pos, Point[i].Vel);
		}
	}
	inter.normalize();
	return inter;
}

Interpolator<vec3> DataCamera::BuildAngInterpolator() const
{
	Interpolator<vec3> inter(Interpolator<vec3>::TYPE_ANGULAR_LERP);
	if (Point.num > 0)
		if (Point[0].Type == CPKCamFlight)
			inter.jump(v_0, v_0);
	for (int i=0; i<Point.num; i++){
		if (Point[i].Type == CPKCamFlight){
			inter.add(Point[i].Ang, Point[i].Duration);
		}else if (Point[i].Type == CPKSetCamPosAng){
			inter.jump(Point[i].Ang, v_0);
		}
	}
	inter.normalize();
	return inter;
}

