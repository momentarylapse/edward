/*
 * DataCamera.cpp
 *
 *  Created on: 30.09.2012
 *      Author: michi
 */

#include "DataCamera.h"
#include "../../Edward.h"

DataCamera::DataCamera() :
	Data(FD_CAMERAFLIGHT)
{
}

DataCamera::~DataCamera()
{
}

void DataCamera::reset()
{
	filename = "";
	Point.clear();
	Vel.clear();
	resetHistory();
	notify();
}

bool DataCamera::load(const string& _filename, bool deep)
{
	bool Error = false;
	reset();

	filename = _filename;
	File *f = FileOpen(filename);
	if (!f){
		ed->setMessage("nicht lesbar");
		return false;
	}
	int ffv=f->ReadFileFormatVersion();
	if (ffv == 2){

		f->ReadComment();
		int n = f->ReadInt();
		//msg_write(NumCamPoints);
		for (int i=0;i<n;i++){
			WorldCamPoint c;
			memset(&c, 0, sizeof(WorldCamPoint));
			f->ReadComment();
			c.Type=f->ReadInt();
			if (c.Type == CPKSetCamPos){
				f->ReadVector(&c.pos);
				c.Duration = f->ReadFloat();
			}else if (c.Type == CPKSetCamPosRel){
				f->ReadVector(&c.pos);
				c.Duration = f->ReadFloat();
			}else if (c.Type == CPKSetCamAng){
				f->ReadVector(&c.Ang);
				c.Duration = f->ReadFloat();
			}else if (c.Type == CPKSetCamPosAng){
				f->ReadVector(&c.pos);
				f->ReadVector(&c.Ang);
				c.Duration = f->ReadFloat();
			}else if (c.Type == CPKCamFlight){
				f->ReadVector(&c.pos);
				f->ReadVector(&c.Vel);
				f->ReadVector(&c.Ang);
				c.Duration = f->ReadFloat();
			}
			if ((c.Type == CPKSetCamPosAng) && (c.Duration > 0)){
				WorldCamPoint c2 = c;
				c2.Duration = 0;
				Point.add(c2);
				c.Type = CPKCamFlight;
			}
			Point.add(c);
		}

		f->Close();
	}else{
		ed->errorBox(format(_("Falsches Dateiformat der Datei %s: %d (statt %d)!"), filename.c_str(), ffv, 2));
		Error=true;
	}
	delete(f);
	UpdateVel();
	resetHistory();
	notify();
	return !Error;
}

bool DataCamera::save(const string& _filename)
{
	filename = _filename;
	File *f = FileCreate(filename);
	f->FloatDecimals = 4;
	f->WriteFileFormatVersion(false, 2);

	f->WriteComment("// Number Of CamPoints");
	f->WriteInt(Point.num);
	foreachi(WorldCamPoint &c, Point, i){
		f->WriteComment(format("// Point No.%d", i));
		f->WriteInt(c.Type);
		if (c.Type == CPKSetCamPos){
			f->WriteVector(&c.pos);
			f->WriteFloat(c.Duration);
		}else if (c.Type == CPKSetCamPosRel){
			f->WriteVector(&c.pos);
			f->WriteFloat(c.Duration);
		}else if (c.Type == CPKSetCamAng){
			f->WriteVector(&c.Ang);
			f->WriteFloat(c.Duration);
		}else if (c.Type == CPKSetCamPosAng){
			f->WriteVector(&c.pos);
			f->WriteVector(&c.Ang);
			f->WriteFloat(c.Duration);
		}else if (c.Type == CPKCamFlight){
			f->WriteVector(&c.pos);
			f->WriteVector(&c.Vel);
			f->WriteVector(&c.Ang);
			f->WriteFloat(c.Duration);
		}
	}
	f->WriteComment("#");

	delete(f);
	ed->setMessage(_("Kamera-Script gespeichert!"));
	action_manager->markCurrentAsSave();
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

Interpolator<vector> DataCamera::BuildPosInterpolator() const
{
	Interpolator<vector> inter(Interpolator<vector>::TYPE_CUBIC_SPLINE);
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

Interpolator<vector> DataCamera::BuildAngInterpolator() const
{
	Interpolator<vector> inter(Interpolator<vector>::TYPE_ANGULAR_LERP);
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

