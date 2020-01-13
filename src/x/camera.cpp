/*----------------------------------------------------------------------------*\
| Camera                                                                       |
| -> representing the camera (view port)                                       |
| -> can be controlled by a camera script                                      |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2007.12.23 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#include "../lib/nix/nix.h"
#include "camera.h"
#include "../meta.h"



#define CPKSetCamPos	0
#define CPKSetCamPosRel	1
#define CPKSetCamAng	2
#define CPKSetCamPosAng	4
#define CPKCamFlight	10
#define CPKCamFlightRel	11



Array<Camera*> cameras;
Camera *cam; // "camera"
Camera *cur_cam; // currently rendering

void ExecuteCamPoint(Camera *cam);

namespace nix{
extern matrix view_matrix, projection_matrix;
}


void CameraInit()
{
	CameraReset();
}

void CameraReset()
{
	xcon_del(cameras);

	// create the main-view ("cam")
	cam = new Camera(v_0, quaternion::ID, rect::ID);
	cur_cam = cam;
}

void Camera::reset()
{
	pmvd.opaque.clear();
	pmvd.trans.clear();

	zoom = 1.0f;
	scale_x = 1;
	z = 0.999999f;
	min_depth = 1.0f;
	max_depth = 100000.0f;
	output = NULL;
	input = NULL;
	shader = NULL;
	shaded_displays = true;

	enabled = false;
	dest = rect::ID;
	cam_point_nr = -1;
	cam_point.clear();

	show = false;
	
	pos = vel = rot = v_0;
	ang = quaternion::ID;
	last_pos = view_pos = vel_rt = pos_0 = vel_0 = ang_0 = pos_1 = vel_1 = ang_1 = v_0;
	a_pos = b_pos = a_ang = b_ang = v_0;
	script_rot_0 = script_rot_1 = v_0;
	script_ang[0] = script_ang[1] = v_0;
	el = el_rt = flight_time = flight_time_el = 0;
	post_projection_matrix = NULL;
	modal = false;
	automatic = false;
	real_time = false;
	jump_to_pos = false;
	auto_over = false;

	m_all = matrix::ID;
	im_all = matrix::ID;
}

Camera::Camera()
{
	reset();
	enabled = true;
	show = true;
	auto_over = -1;
	cam_point_nr = -1;
	jump_to_pos = false;

	// register
	xcon_reg(this, cameras);
}

Camera::Camera(const vector &_pos, const quaternion &_ang, const rect &_dest) : Camera()
{
	pos = _pos;
	ang = _ang;
	dest = _dest;
}

Camera::~Camera()
{
	// unregister
	xcon_unreg(this, cameras);
}


void Camera::__init__()
{
	new(this) Camera;
}

void Camera::__init_ext__(const vector &_pos, const quaternion &_ang, const rect &_dest)
{
	new(this) Camera(_pos, _ang, _dest);
}

void Camera::__delete__()
{
	//this->~Camera();
	// unregister
	for (int i=0;i<cameras.num;i++)
		if (cameras[i] == this)
			cameras.erase(i);
}

void SetAim(Camera *view,vector &pos,vector &vel,vector &ang,float time,bool real_time)
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
	view->ang_0 = view->ang.get_angles();
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

void Camera::StartScript(const string &filename,const vector &dpos)
{
	if (filename.num <= 0){
		automatic=false;
		return;
	}
	msg_write("loading camera script: " + filename);
	msg_right();
	cam_point_nr = -1;


	File *f = FileOpenText(ScriptDir + filename + ".camera");
	float x,y,z;
	if (f){
		int ffv=f->ReadFileFormatVersion();
		if (ffv!=2){
			f->close();
			delete(f);
			msg_error(format("wrong file format: %d (2 expected)", ffv));
		}
		cam_point.clear();
		f->read_comment();
		int n = f->read_int();
		for (int ip=0;ip<n;ip++){
			CamPoint p;
			f->read_comment();
			p.type = f->read_int();
			if (p.type == CPKSetCamPos){
				x  =f->read_float();
				y  =f->read_float();
				z = f->read_float();
				p.pos = vector(x, y, z) + dpos;
				p.duration = f->read_float();
			}else if (p.type == CPKSetCamPosRel){
				x  =f->read_float();
				y  =f->read_float();
				z = f->read_float();
				p.pos = vector(x, y, z) + dpos;
				p.duration = f->read_float();
			}else if (p.type == CPKSetCamAng){
				x  =f->read_float();
				y  =f->read_float();
				z = f->read_float();
				p.ang  = vector(x, y, z);
				p.duration = f->read_float();
			}else if (p.type == CPKSetCamPosAng){
				x = f->read_float();
				y = f->read_float();
				z = f->read_float();
				p.pos = vector( x, y, z) + dpos;
				x  =f->read_float();
				y  =f->read_float();
				z = f->read_float();
				p.ang  = vector(x, y, z);
				p.duration = f->read_float();
			}else if (p.type == CPKCamFlight){
				x = f->read_float();
				y = f->read_float();
				z = f->read_float();
				p.pos = vector( x, y, z) + dpos;
				x = f->read_float();
				y = f->read_float();
				z = f->read_float();
				p.vel = vector( x, y, z);
				x = f->read_float();
				y = f->read_float();
				z = f->read_float();
				p.ang = vector( x, y, z);
				p.duration = f->read_float();
			}
			cam_point.add(p);
		}
		FileClose(f);
		cam_point_nr = 0;
		flight_time_el = flight_time = 0;
		ExecuteCamPoint(this);
	}
	vel = vel_rt = v_0;
	msg_left();
}

void Camera::StopScript()
{
	cam_point_nr = -1;
	cam_point.clear();
	automatic = false;
}

void ExecuteCamPoint(Camera *view)
{
	view->script_ang[0] = view->ang.get_angles();
	if (view->cam_point_nr == 0)
		view->script_rot_1 = v_0;
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
		view->ang = quaternion::rotation_v(p->ang);
		view->script_rot_1 = v_0;
		view->cam_point_nr ++;
		ExecuteCamPoint(view);
		return;
	}else if (p->type == CPKSetCamPosAng){
		view->pos = p->pos;
		view->ang = quaternion::rotation_v(p->ang);
		view->script_rot_1 = v_0;
		if (p->duration <= 0){
			view->cam_point_nr ++;
			ExecuteCamPoint(view);
			return;
		}else{
			SetAim(	view,
					p->pos, p->vel, p->ang,
					p->duration,
					true);
			view->a_pos = v_0; //view->pos;
			view->b_pos = v_0; //view->pos;
			view->pos_0 = view->pos;
			view->pos_1 = view->pos;
			view->vel_0 = v_0;
			view->vel_1 = v_0;
			view->a_ang = v_0; //view->ang;
			view->b_ang = v_0; //view->ang;
			view->ang_0 = view->ang.get_angles();
			view->ang_1 = view->ang.get_angles();
			view->rot = v_0;
			view->script_rot_0 = v_0;
			view->script_rot_1 = v_0;
		}
	}else if (p->type == CPKCamFlight){
		float ft = view->flight_time_el - view->flight_time;
		if (view->cam_point_nr>0){
			view->pos = view->cam_point[view->cam_point_nr - 1].pos;
			view->vel = view->cam_point[view->cam_point_nr - 1].vel;
			view->ang = quaternion::rotation_v(view->cam_point[view->cam_point_nr - 1].ang);
		}
		SetAim(	view,
				p->pos, p->vel, p->ang,
				p->duration,
				true);
		view->flight_time_el = ft;
		// create data for the rotation
		view->script_rot_0 = v_0;
		if (view->cam_point_nr > 0)
			if (view->cam_point[view->cam_point_nr - 1].type == CPKCamFlight)
				view->script_rot_0 = view->script_rot_1;//(view->cam_point[view->cam_pointNr].Ang-view->ScriptAng[1])/(view->cam_point[view->cam_pointNr-1].Wait + p->wait);
		view->script_rot_1 = v_0;
		if (view->cam_point_nr < view->cam_point.num - 1)
			if (view->cam_point[view->cam_point_nr + 1].type == CPKCamFlight)
				view->script_rot_1 = (view->cam_point[view->cam_point_nr + 1].ang - view->ang_0) / (p->duration + view->cam_point[view->cam_point_nr + 1].duration);
		float dt = p->duration;
		view->a_ang = (view->script_rot_1+view->script_rot_0)/dt/dt - 2*(view->ang_1-view->ang_0)/dt/dt/dt;
		view->b_ang = 3*(view->ang_1-view->ang_0)/dt/dt - (view->script_rot_1 + 2*view->script_rot_0)/dt;
	}
	view->script_ang[1] = view->script_ang[0];
}

void Camera::OnIterate(float dt)
{
	// ???
	if (auto_over >= 0)
		auto_over ++;
	if (auto_over >= 4){
		auto_over = -1;
		//ang = prae_ang;
	}

	if (automatic){

		// script-controlled camera animation
		auto_over = 0;
		if (real_time)
			flight_time_el += Engine.ElapsedRT;
		else
			flight_time_el += Engine.Elapsed;
		if (flight_time_el >= flight_time){
			//Pos=Pos1;
			//Vel=Vel1;
			if (real_time){
				vel_rt = vel_1;
				vel = vel_rt / Engine.TimeScale;
			}else{
				vel = vel_1;
				vel_rt = vel * Engine.TimeScale;
			}
			ang = quaternion::rotation_v(ang_1);
			if (cam_point_nr >= 0){
				cam_point_nr ++;
				ExecuteCamPoint(this);
			}
			if (cam_point_nr < 0){ // even(!), if the last CamPoint was the last one in the script
				automatic = false;
			}
			auto_over = 1;
		}else{
			float t=flight_time_el;
			float t2=t*t; // t^2
			float t3=t2*t; // t^3
			// cubic position interpolation
			pos = a_pos*t3 + b_pos*t2 + vel_0*t + pos_0;
			if (real_time){
				vel_rt = vel_0 + 3*a_pos*t2 + 2*b_pos*t;
				vel = vel_rt / Engine.TimeScale;
			}else{
				vel = vel_0 + 3*a_pos*t2 + 2*b_pos*t;
				vel_rt = vel * Engine.TimeScale;
			}
			// linear angular interpolation
			//View[i]->Ang=VecAngInterpolate(    ...View[i]->Ang0 + View[i]->Rot*t;
			// cubic angular interpolation
			ang= quaternion::rotation_v(a_ang*t3 + b_ang*t2 + script_rot_0*t + ang_0);
		}
	}else{

		float elapsed = Engine.Elapsed;
		if (elapsed==0)
			elapsed=0.000001f;
		vel = (pos - last_pos) / elapsed;
		vel_rt = (pos - last_pos) / Engine.ElapsedRT;
	}
	if (jump_to_pos)
		vel = vel_rt = v_0;
	last_pos = pos;
	jump_to_pos = false;
}

void CameraCalcMove(float dt)
{
	for(Camera *v: cameras){
		if (!v->enabled)
			continue;
		v->OnIterate(dt);
	}
}

void Camera::Start()
{
	cur_cam = this;
	if (output){
		nix::StartIntoTexture(output);
	}else{
		nix::Scissor(rect((float)nix::target_width * dest.x1,
					(float)nix::target_width * dest.x2,
					(float)nix::target_height * dest.y1,
					(float)nix::target_height * dest.y2));
	}
}

int num_used_clipping_planes = 0;

void Camera::SetView()
{
	// Kamera-Position der Ansicht
	// im Spiegel: Pos!=ViewPos...
	view_pos = pos;
	
	for (int i=0;i<num_used_clipping_planes;i++)
		nix::EnableClipPlane(i, false);

	// View-Transformation setzen (Kamera)
	float center_x = (float)nix::target_width * (dest.x1 + dest.x2) / 2;
	float center_y = (float)nix::target_height * (dest.y1 + dest.y2) / 2;
	float height = (float)nix::target_height * (dest.y2 - dest.y1) / zoom;
	nix::SetProjectionPerspectiveExt(center_x, center_y, height * scale_x, height, min_depth, max_depth);
	nix::SetViewPosAng(view_pos, ang);
	
	m_all = (nix::projection_matrix * nix::view_matrix).inverse();

	// clipping planes
	for (int i=0;i<clipping_plane.num;i++){
		nix::SetClipPlane(i, clipping_plane[i]);
		nix::EnableClipPlane(i, true);
	}
	/*for (int i=ClippingPlane.num;i<num_used_clipping_planes;i++)
		NixEnableClipPlane(i, false);*/
	num_used_clipping_planes = clipping_plane.num;
}

void Camera::SetViewLocal()
{
	// Kamera-Position der Ansicht
	// im Spiegel: Pos!=ViewPos...
	view_pos = pos;

	// View-Transformation setzen (Kamera)
	float center_x = (float)nix::target_width * (dest.x1 + dest.x2) / 2;
	float center_y = (float)nix::target_height * (dest.y1 + dest.y2) / 2;
	float height = (float)nix::target_height * (dest.y2 - dest.y1) / zoom;
	nix::SetProjectionPerspectiveExt(center_x, center_y, height * scale_x, height, 0.01f, 1000000.0f);
	nix::SetViewPosAng(v_0, ang);
	
	m_all = (nix::projection_matrix * nix::view_matrix).inverse();
}

vector Camera::Project(const vector &v)
{
	float x = m_all._00 * v.x + m_all._01 * v.y + m_all._02 * v.z + m_all._03;
	float y = m_all._10 * v.x + m_all._11 * v.y + m_all._12 * v.z + m_all._13;
	float z = m_all._20 * v.x + m_all._21 * v.y + m_all._22 * v.z + m_all._23;
	float w = m_all._30 * v.x + m_all._31 * v.y + m_all._32 * v.z + m_all._33;
	if (w <= 0)
		return vector(0, 0, -1);
	return vector(x/w * 0.5f + 0.5f, 0.5f - y/w * 0.5f, z/w * 0.5f + 0.5f);
}

vector Camera::Unproject(const vector &v)
{
	float xx = (v.x - 0.5f) * 2;
	float yy = (0.5f - v.y) * 2;
	float zz = (v.z - 0.5f) * 2;
	float x = im_all._00 * xx + im_all._01 * yy + im_all._02 * zz + im_all._03;
	float y = im_all._10 * xx + im_all._11 * yy + im_all._12 * zz + im_all._13;
	float z = im_all._20 * xx + im_all._21 * yy + im_all._22 * zz + im_all._23;
	float w = im_all._30 * xx + im_all._31 * yy + im_all._32 * zz + im_all._33;
	return vector(x, y, z) / w;
}

void CameraShiftAll(const vector &dpos)
{
	for (Camera *c: cameras)
		c->pos += dpos;
}

