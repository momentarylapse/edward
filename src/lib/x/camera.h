/*----------------------------------------------------------------------------*\
| Camera                                                                       |
| -> representing the camera (view port)                                       |
| -> can be controlled by a camera script                                      |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2009.11.22 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#if !defined(CAMERA_H__INCLUDED_)
#define CAMERA_H__INCLUDED_


struct CamPoint
{
	int type;
	float duration;
	vector pos, vel, ang;
};

struct PartialModelView
{
	void *p;
	float z;
	int detail;

	bool operator < (const PartialModelView &pmv) const
	{	return z < pmv.z;	}
};

struct PartialModelViewData
{
	Array<PartialModelView> opaque;
	Array<PartialModelView> trans;
};

class CView : public XContainer
{
public:
	CView();
	void reset();
	
	rect dest;
	float z;
	int cam_point_nr;
	Array<CamPoint> cam_point;

	bool show;
	int output_texture, input_texture;
	int shader;
	bool shaded_displays;
	
	vector last_pos, pos, view_pos, vel, vel_rt, ang, rot, pos_0, vel_0, ang_0, pos_1, vel_1, ang_1, a_pos, b_pos, a_ang, b_ang, script_rot_0, script_rot_1;
	vector script_ang[2];
	float zoom, el, el_rt, flight_time, flight_time_el;
	matrix *post_projection_matrix;
	bool modal, automatic, real_time, jump_to_pos;
	int auto_over;

	PartialModelViewData pmvd;

	void Start();
	void SetView();
	void SetViewLocal();
	// camera scripts
	void StartScript(const string &filename, const vector &dpos);
	void StopScript();

	Array<plane> clipping_plane;
	Array<CModel*> ignore;
};

void CameraInit();
void CameraReset();
void CameraCalcMove();
CView *_cdecl CameraCreateView(const vector &pos, const vector &ang, const rect &dest, bool show);
void CameraDeleteView(CView *view);

extern Array<CView*> View;
extern CView *Cam; // "camera"
extern CView *view_cur; // currently rendering

#endif

