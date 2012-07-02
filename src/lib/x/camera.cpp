/*----------------------------------------------------------------------------*\
| Camera                                                                       |
| -> representing the camera (view port)                                       |
| -> can be controlled by a camera script                                      |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2007.12.23 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#include "x.h"



#define CPKSetCamPos	0
#define CPKSetCamPosRel	1
#define CPKSetCamAng	2
#define CPKSetCamPosAng	4
#define CPKCamFlight	10
#define CPKCamFlightRel	11



Array<CView*> View;
CView *Cam; // "camera"
CView *view_cur; // currently rendering

void ExecuteCamPoint(CView *view);


void CameraInit()
{
	CameraReset();
}

void CameraReset()
{
	msg_db_r("CameraReset",1);
	foreach(View, v)
		delete(v);
	View.clear();

	// create the main-view ("cam")
	Cam = CameraCreateView(v0, v0, r_id, true);
	view_cur = Cam;

	msg_db_l(1);
}

void CView::reset()
{
	pmvd.opaque.clear();
	pmvd.trans.clear();

	zoom = 1.0f;
	z = 0.999999f;
	output_texture = -1;
	input_texture = -1;
	shader = -1;
	shaded_displays = true;

	enabled = false;
	dest = rect(0, 0, 0, 0);
	cam_point_nr = -1;
	cam_point.clear();

	show = false;
	
	pos = vel = ang = rot = v0;
	last_pos = view_pos = vel_rt = pos_0 = vel_0 = ang_0 = pos_1 = vel_1 = ang_1 = v0;
	a_pos = b_pos = a_ang = b_ang = v0;
	script_rot_0 = script_rot_1 = v0;
	script_ang[0] = script_ang[1] = v0;
	el = el_rt = flight_time = flight_time_el = 0;
	post_projection_matrix = NULL;
	modal = false;
	automatic = false;
	real_time = false;
	jump_to_pos = false;
	auto_over = false;
}

CView::CView()
{
	this->reset();
	enabled = true;
	show = true;
	auto_over = -1;
	cam_point_nr = -1;
	jump_to_pos = false;
}

CView *CameraCreateView(const vector &pos,const vector &ang,const rect &dest,bool show)
{
	msg_db_r("CameraCreateView",1);
	xcont_find_new(XContainerView, CView, v, View);
	// initial data
	v->pos = pos;
	v->ang = ang;
	v->dest = dest;
	v->show = show;

	msg_db_l(1);
	return v;
}

void CameraDeleteView(CView *v)
{
	v->enabled = false;
	v->show = false;
	v->used = false;
}

void SetAim(CView *view,vector &pos,vector &vel,vector &ang,float time,bool real_time)
{
	//if (view->AutoOver<0)
	//	view->PraeAng=view->Ang;
	view->real_time = real_time;
	view->flight_time = time;
	view->flight_time_el = 0;
	view->pos_0 = view->pos;
	if (real_time)
		view->vel_0 = view->vel_rt;
	else
		view->vel_0 = view->vel;
	view->ang_0 = view->ang;
	view->pos_1 = pos;
	view->vel_1 = vel;
	view->ang_1 = ang;
	view->rot = (view->ang_1 - view->ang_0) / time;
	view->a_pos = (time*view->vel_1 - 2 * (view->pos_1 - view->pos_0) + view->vel_0 * time) / (float)pow(time, 3);
	view->b_pos = ((view->pos_1 - view->pos_0) - view->a_pos * (float)pow(time, 3) - view->vel_0 * time) / (time * time);
	if (view->ang_1.x - view->ang_0.x > pi)	view->ang_0.x += 2*pi;
	if (view->ang_0.x - view->ang_1.x > pi)	view->ang_0.x -= 2*pi;
	if (view->ang_1.y - view->ang_0.y > pi)	view->ang_0.y += 2*pi;
	if (view->ang_0.y - view->ang_1.y > pi)	view->ang_0.y -= 2*pi;
	if (view->ang_1.z - view->ang_0.z > pi)	view->ang_0.z += 2*pi;
	if (view->ang_0.z - view->ang_1.z > pi)	view->ang_0.z -= 2*pi;
	view->rot = (view->ang_1 - view->ang_0) / time;
	view->automatic = true;
}

void CView::StartScript(const string &filename,const vector &dpos)
{
	if (filename.num <= 0){
		automatic=false;
		return;
	}
	msg_write("loading camera script: " + filename);
	msg_right();
	cam_point_nr = -1;


	CFile *f = OpenFile(ScriptDir + filename + ".camera");
	float x,y,z;
	if (f){
		int ffv=f->ReadFileFormatVersion();
		if (ffv!=2){
			f->Close();
			delete(f);
			msg_error(format("wrong file format: %d (2 expected)", ffv));
		}
		cam_point.clear();
		int n = f->ReadIntC();
		for (int ip=0;ip<n;ip++){
			CamPoint p;
			p.type = f->ReadIntC();
			if (p.type == CPKSetCamPos){
				x  =f->ReadFloat();
				y  =f->ReadFloat();
				z = f->ReadFloat();
				p.pos = vector(x, y, z) + dpos;
				p.duration = f->ReadFloat();
			}else if (p.type == CPKSetCamPosRel){
				x  =f->ReadFloat();
				y  =f->ReadFloat();
				z = f->ReadFloat();
				p.pos = vector(x, y, z) + dpos;
				p.duration = f->ReadFloat();
			}else if (p.type == CPKSetCamAng){
				x  =f->ReadFloat();
				y  =f->ReadFloat();
				z = f->ReadFloat();
				p.ang  = vector(x, y, z);
				p.duration = f->ReadFloat();
			}else if (p.type == CPKSetCamPosAng){
				x = f->ReadFloat();
				y = f->ReadFloat();
				z = f->ReadFloat();
				p.pos = vector( x, y, z) + dpos;
				x  =f->ReadFloat();
				y  =f->ReadFloat();
				z = f->ReadFloat();
				p.ang  = vector(x, y, z);
				p.duration = f->ReadFloat();
			}else if (p.type == CPKCamFlight){
				x = f->ReadFloat();
				y = f->ReadFloat();
				z = f->ReadFloat();
				p.pos = vector( x, y, z) + dpos;
				x = f->ReadFloat();
				y = f->ReadFloat();
				z = f->ReadFloat();
				p.vel = vector( x, y, z);
				x = f->ReadFloat();
				y = f->ReadFloat();
				z = f->ReadFloat();
				p.ang = vector( x, y, z);
				p.duration = f->ReadFloat();
			}
			cam_point.add(p);
		}
		FileClose(f);
		cam_point_nr = 0;
		flight_time_el = flight_time = 0;
		ExecuteCamPoint(this);
	}
	vel = vel_rt = v0;
	msg_left();
}

void CView::StopScript()
{
	cam_point_nr = -1;
	cam_point.clear();
	automatic = false;
}

void ExecuteCamPoint(CView *view)
{
	view->script_ang[0] = view->ang;
	if (view->cam_point_nr == 0)
		view->script_rot_1 = v0;
	if (view->cam_point_nr >= view->cam_point.num){
		view->cam_point_nr = -1;
		view->cam_point.clear();
		return;
	}
	CamPoint *p = &view->cam_point[view->cam_point_nr];
	if (p->type == CPKSetCamPos){
		view->pos = p->pos;
		view->cam_point_nr ++;
		ExecuteCamPoint(view);
		return;
	}else if (p->type == CPKSetCamPosRel){
		view->pos += p->pos;
		view->cam_point_nr ++;
		ExecuteCamPoint(view);
		return;
	}else if (p->type == CPKSetCamAng){
		view->ang = p->ang;
		view->script_rot_1 = v0;
		view->cam_point_nr ++;
		ExecuteCamPoint(view);
		return;
	}else if (p->type == CPKSetCamPosAng){
		view->pos = p->pos;
		view->ang = p->ang;
		view->script_rot_1 = v0;
		if (p->duration <= 0){
			view->cam_point_nr ++;
			ExecuteCamPoint(view);
			return;
		}else{
			SetAim(	view,
					p->pos, p->vel, p->ang,
					p->duration,
					true);
			view->a_pos = v0; //view->pos;
			view->b_pos = v0; //view->pos;
			view->pos_0 = view->pos;
			view->pos_1 = view->pos;
			view->vel_0 = v0;
			view->vel_1 = v0;
			view->a_ang = v0; //view->ang;
			view->b_ang = v0; //view->ang;
			view->ang_0 = view->ang;
			view->ang_1 = view->ang;
			view->rot = v0;
			view->script_rot_0 = v0;
			view->script_rot_1 = v0;
		}
	}else if (p->type == CPKCamFlight){
		float ft = view->flight_time_el - view->flight_time;
		if (view->cam_point_nr>0){
			view->pos = view->cam_point[view->cam_point_nr - 1].pos;
			view->vel = view->cam_point[view->cam_point_nr - 1].vel;
			view->ang = view->cam_point[view->cam_point_nr - 1].ang;
		}
		SetAim(	view,
				p->pos, p->vel, p->ang,
				p->duration,
				true);
		view->flight_time_el = ft;
		// create data for the rotation
		view->script_rot_0 = v0;
		if (view->cam_point_nr > 0)
			if (view->cam_point[view->cam_point_nr - 1].type == CPKCamFlight)
				view->script_rot_0 = view->script_rot_1;//(view->cam_point[view->cam_pointNr].Ang-view->ScriptAng[1])/(view->cam_point[view->cam_pointNr-1].Wait + p->wait);
		view->script_rot_1 = v0;
		if (view->cam_point_nr < view->cam_point.num - 1)
			if (view->cam_point[view->cam_point_nr + 1].type == CPKCamFlight)
				view->script_rot_1 = (view->cam_point[view->cam_point_nr + 1].ang - view->ang_0) / (p->duration + view->cam_point[view->cam_point_nr + 1].duration);
		float dt = p->duration;
		view->a_ang = (view->script_rot_1+view->script_rot_0)/dt/dt - 2*(view->ang_1-view->ang_0)/dt/dt/dt;
		view->b_ang = 3*(view->ang_1-view->ang_0)/dt/dt - (view->script_rot_1 + 2*view->script_rot_0)/dt;
	}
	view->script_ang[1] = view->script_ang[0];
}

void CameraCalcMove()
{
	msg_db_r("CamCalcMove",2);

	foreach(View, v){
		if (!v->enabled)
			continue;

		// ???
		if (v->auto_over >= 0)
			v->auto_over ++;
		if (v->auto_over >= 4){
			v->auto_over = -1;
			//v->ang = v->prae_ang;
		}

		if (v->automatic){

		// script-controlled camera animation
			v->auto_over = 0;
			if (v->real_time)
				v->flight_time_el += ElapsedRT;
			else
				v->flight_time_el += Elapsed;
			if (v->flight_time_el >= v->flight_time){
				//v->Pos=v->Pos1;
				//v->Vel=v->Vel1;
				if (v->real_time){
					v->vel_rt = v->vel_1;
					v->vel = v->vel_rt / TimeScale;
				}else{
					v->vel = v->vel_1;
					v->vel_rt = v->vel * TimeScale;
				}
				v->ang = v->ang_1;
				if (v->cam_point_nr >= 0){
					v->cam_point_nr ++;
					ExecuteCamPoint(v);
				}
				if (v->cam_point_nr < 0){ // even(!), if the last CamPoint was the last one in the script
					v->automatic = false;
				}
				v->auto_over = 1;
			}else{
				float t=v->flight_time_el;
				float t2=t*t; // t^2
				float t3=t2*t; // t^3
				// cubic position interpolation
				v->pos = v->a_pos*t3 + v->b_pos*t2 + v->vel_0*t + v->pos_0;
				if (v->real_time){
					v->vel_rt = v->vel_0 + 3*v->a_pos*t2 + 2*v->b_pos*t;
					v->vel = v->vel_rt / TimeScale;
				}else{
					v->vel = v->vel_0 + 3*v->a_pos*t2 + 2*v->b_pos*t;
					v->vel_rt = v->vel*TimeScale;
				}
				// linear angular interpolation
				//View[i]->Ang=VecAngInterpolate(    ...View[i]->Ang0 + View[i]->Rot*t;
				// cubic angular interpolation
				v->ang= v->a_ang*t3 + v->b_ang*t2 + v->script_rot_0*t + v->ang_0;
			}
		}else{

			float elapsed=Elapsed;
			if (elapsed==0)
				elapsed=0.000001f;
			v->vel = (v->pos - v->last_pos)/elapsed;
			v->vel_rt = (v->pos - v->last_pos)/ElapsedRT;
		}
		if (v->jump_to_pos)
			v->vel = v->vel_rt = v0;
		v->last_pos = v->pos;
		v->jump_to_pos = false;
	}

	msg_db_l(2);
}

void CView::Start()
{
	view_cur = this;
	if (output_texture >= 0){
		NixStart(output_texture);
	}else{
		NixStartPart(	int((float)MaxX * dest.x1),
						int((float)MaxY * dest.y1),
						int((float)MaxX * dest.x2),
						int((float)MaxY * dest.y2),true);
	}
}

int num_used_clipping_planes = 0;

void CView::SetView()
{
	vector scale = vector(zoom, zoom, 1);

	// Kamera-Position der Ansicht
	// im Spiegel: Pos!=ViewPos...
	view_pos = pos;
	
	for (int i=0;i<num_used_clipping_planes;i++)
		NixEnableClipPlane(i, false);

	// View-Transformation setzen (Kamera)
	NixSetView(true, view_pos, ang, scale);

	// clipping planes
	for (int i=0;i<clipping_plane.num;i++){
		NixSetClipPlane(i, clipping_plane[i]);
		NixEnableClipPlane(i, true);
	}
	/*for (int i=ClippingPlane.num;i<num_used_clipping_planes;i++)
		NixEnableClipPlane(i, false);*/
	num_used_clipping_planes = clipping_plane.num;
}

void CView::SetViewLocal()
{
	vector scale = vector(zoom, zoom, 1);

	// Kamera-Position der Ansicht
	// im Spiegel: Pos!=ViewPos...
	view_pos = pos;

	// View-Transformation setzen (Kamera)
	NixSetView(true, v0, ang, scale);
}

