/*
 * DataCamera.cpp
 *
 *  Created on: 30.09.2012
 *      Author: michi
 */

#include "DataCamera.h"
#include "../../Edward.h"

DataCamera::DataCamera() :
	Data(FDCameraFlight)
{
}

DataCamera::~DataCamera()
{
}

void DataCamera::Reset()
{
	filename = "";
	Point.clear();
	Vel.clear();
	ResetHistory();
	Notify("Change");
}

bool DataCamera::Load(const string& _filename, bool deep)
{
	bool Error = false;
	Reset();

	filename = _filename;
	CFile *f = OpenFile(filename);
	if (!f){
		ed->SetMessage("nicht lesbar");
		return false;
	}
	int ffv=f->ReadFileFormatVersion();
	if (ffv == 2){

		int n = f->ReadIntC();
		//msg_write(NumCamPoints);
		for (int i=0;i<n;i++){
			WorldCamPoint c;
			memset(&c, 0, sizeof(WorldCamPoint));
			c.Type=f->ReadIntC();
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
			Point.add(c);
		}

		f->Close();
	}else{
		ed->ErrorBox(format(_("Falsches Dateiformat der Datei %s: %d (statt %d)!"), filename.c_str(), ffv, 2));
		Error=true;
	}
	delete(f);
	UpdateVel();
	ResetHistory();
	Notify("Change");
	return !Error;
}

bool DataCamera::Save(const string& _filename)
{
	filename = _filename;
	CFile *f = CreateFile(filename);
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
	ed->SetMessage(_("Kamera-Script gespeichert!"));
	action_manager->MarkCurrentAsSave();
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

