/*
 * PluginManager.cpp
 *
 *  Created on: 01.07.2012
 *      Author: michi
 */

#include "PluginManager.h"

#include "../lib/kaba/kaba.h"
#include "../Edward.h"
#include "../MultiView/MultiView.h"
#include "../MultiView/ActionController.h"
#include "../MultiView/Window.h"
#include "../Mode/Model/ModeModel.h"
#include "../Mode/World/ModeWorld.h"
#include "../Data/Model/Geometry/GeometryBall.h"
#include "../Data/Model/Geometry/GeometryCube.h"
#include "../Data/Model/Geometry/GeometryCylinder.h"
#include "../Data/Model/Geometry/GeometryPlane.h"
#include "../Data/Model/Geometry/GeometryPlatonic.h"
#include "../Data/Model/Geometry/GeometrySphere.h"
#include "../Data/Model/Geometry/GeometryTeapot.h"
#include "../Data/Model/Geometry/GeometryTorus.h"
#include "../Data/Model/Geometry/GeometryTorusKnot.h"

//#define HAS_LIB_PSMOVEAPI 1


#if HAS_LIB_PSMOVEAPI

//#include <psmoveapi/psmoveapi.h>
#include <psmoveapi/psmove.h>
#include <psmoveapi/psmove_tracker.h>
#include <psmoveapi/psmove_fusion.h>


PSMoveTracker *m_tracker = NULL;
PSMoveFusion *m_fusion = NULL;

struct MyPSMoveController {
	PSMove* psmove;
	vector pos, dpos, pos_last;
	vector acc, rot, mag;
	color col;
	quaternion ang, dang;
	float trigger, rumble;
	bool button[8];
	MyPSMoveController() {}
	MyPSMoveController(PSMove *m) {
		psmove = m;
		col = Black;
		rumble = 0;
		trigger = 0;
		ang = quaternion::ID;
		dang = quaternion::ID;
	}

	void update() {
		if (!psmove)
			return;

		while (psmove_poll(psmove)) {}

		psmove_fusion_get_position(m_fusion, psmove, &pos.x, &pos.y, &pos.z);
		pos.y = - pos.y;

		dpos = pos - pos_last;
		pos_last = pos;

		matrix *mat = (matrix*)psmove_fusion_get_modelview_matrix(m_fusion, psmove);
		matrix s = matrix::scale( 1,  1,  -1);
		quaternion q = quaternion::rotation_m( s * *mat * s);
		dang = ang * q;
		ang = q.bar();
		//ang = quaternion::rotation_m( *mat);

		psmove_set_rumble(psmove, rumble*255);
		trigger = (float)psmove_get_trigger(psmove) / 255.0f;

		psmove_get_accelerometer_frame(psmove, Frame_SecondHalf, &acc.x, &acc.y, &acc.z);
		acc.y = - acc.y;
		psmove_get_gyroscope_frame(psmove, Frame_SecondHalf, &rot.x, &rot.y, &rot.z);
		rot.y = - rot.y;

		int _buttons = psmove_get_buttons(psmove);
		button[0] = ((_buttons & Btn_TRIANGLE) != 0);
		button[1] = ((_buttons & Btn_CIRCLE) != 0);
		button[2] = ((_buttons & Btn_CROSS) != 0);
		button[3] = ((_buttons & Btn_SQUARE) != 0);
		button[4] = ((_buttons & Btn_SELECT) != 0);
		button[5] = ((_buttons & Btn_START) != 0);
		button[6] = ((_buttons & Btn_PS) != 0);
		button[7] = ((_buttons & Btn_MOVE) != 0);
	}

	void reset_orientation()
	{
		psmove_reset_orientation(psmove);
		ang = quaternion::ID;
	}
};
Array<MyPSMoveController> psmoves;

void _psmove_init_tracker()
{
	if (m_tracker)
		return;
	int count = min(1, psmove_count_connected());
	m_tracker = psmove_tracker_new();
	m_fusion = psmove_fusion_new(m_tracker, 1., 1000.);
	psmove_tracker_set_mirror(m_tracker, PSMove_True);
	psmove_tracker_set_exposure(m_tracker, Exposure_LOW);
	for (int i=0; i<count; i++) {
		msg_write("psmove controller found: " + i2s(i));
		PSMove *m = psmove_connect_by_id(i);
		msg_write(p2s(m));

		psmove_enable_orientation(m, PSMove_True);
		if (!psmove_has_orientation(m))
			msg_error("psmove without orientation");

		msg_write("registering for tracking...");
		while (psmove_tracker_enable(m_tracker, m) != Tracker_CALIBRATED);
		msg_write("  ok");
		psmove_reset_orientation(m);
		psmoves.add(MyPSMoveController(m));
	}
}

void _psmove_kill_tracker()
{
	psmoves.clear();
	if (m_fusion)
		psmove_fusion_free(m_fusion);
	if (m_tracker)
		psmove_tracker_free(m_tracker);
}

void _psmove_update_tracker()
{
	if (!m_tracker)
		return;
	for (auto &c: psmoves)
		c.update();

	psmove_tracker_update_image(m_tracker);
	psmove_tracker_update(m_tracker, NULL);
}


#if 0
struct ExampleHandler : public psmoveapi::Handler {
    ExampleHandler() : color{0.f, 0.f, 0.f} , rumble(0.f), interactive(false) {}

    virtual void connect(Controller *controller) {
        printf("Connect: %s\n", controller->serial);
    }

    virtual void update(Controller *controller) {
        if (!interactive) {
            controller->color = color;
            controller->rumble = rumble;
        } else {
            if ((controller->buttons & Btn_TRIANGLE) != 0) {
                //printf("Triangle pressed, with trigger value: %.2f\n", controller->trigger);
                controller->rumble = controller->trigger;
            } else {
                controller->rumble = 0.f;
            }

            controller->color = { 0.f, 0.f, controller->trigger };

            /*printf("Accel: %.2f, %.2f, %.2f\n", controller->accelerometer.x, controller->accelerometer.y, controller->accelerometer.z);
            printf("Gyro: %.2f, %.2f, %.2f\n", controller->gyroscope.x, controller->gyroscope.y, controller->gyroscope.z);
            printf("Magnetometer: %.2f, %.2f, %.2f\n", controller->magnetometer.x, controller->magnetometer.y, controller->magnetometer.z);
            printf("Connection [%c] USB [%c] Bluetooth; Buttons: 0x%04x\n", controller->usb ? 'x' : ' ', controller->bluetooth ? 'x' : ' ', controller->buttons);*/

            psm_controller.trigger = controller->trigger;
            controller->rumble = psm_controller.rumble;
            controller->color = {psm_controller.col.r, psm_controller.col.g, psm_controller.col.b};
            psm_controller.accel = vector(-controller->accelerometer.x, controller->accelerometer.y, controller->accelerometer.z);
            psm_controller.gyro = vector(-controller->gyroscope.x, controller->gyroscope.y, controller->gyroscope.z);
            psm_controller.mag = vector(-controller->magnetometer.x, controller->magnetometer.y, controller->magnetometer.z);
            psm_controller.button[0] = ((controller->buttons & Btn_TRIANGLE) != 0);
            psm_controller.button[1] = ((controller->buttons & Btn_CIRCLE) != 0);
            psm_controller.button[2] = ((controller->buttons & Btn_CROSS) != 0);
            psm_controller.button[3] = ((controller->buttons & Btn_SQUARE) != 0);
            psm_controller.button[4] = ((controller->buttons & Btn_SELECT) != 0);
            psm_controller.button[5] = ((controller->buttons & Btn_START) != 0);
            psm_controller.button[6] = ((controller->buttons & Btn_PS) != 0);
        }
    }

    virtual void disconnect(Controller *controller) {
        printf("Disconnect: %s\n", controller->serial);
    }

    RGB color;
    float rumble;
    bool interactive;
};

ExampleHandler *psmove_handler = NULL;
psmoveapi::PSMoveAPI *psmove_api = NULL;
#endif


#endif





PluginManager::PluginManager()
{
	init();
}

PluginManager::~PluginManager()
{
}

void PluginManager::execute(const string & filename)
{
	Kaba::config.directory = "";
	try{
		Kaba::Script *s = Kaba::Load(filename);
		typedef void func_t();
		func_t *f = (func_t*)s->match_function("main", "void", {});
		if (f)
			f();
	}catch(Kaba::Exception &e){
		ed->error_box(e.message());
	}

	//Kaba::DeleteAllScripts(true, true);
}

#define _offsetof(t, x)	((int)(long)((char*)&(((t*)ppp)->x) - ppp))

hui::Window *GlobalMainWin = ed;

void PluginManager::init() {
	Kaba::init();

	GlobalMainWin = ed;


#if HAS_LIB_PSMOVEAPI
	Kaba::link_external("psmoves", &psmoves);


	Kaba::declare_class_size("PSMoveController", sizeof(MyPSMoveController));
	Kaba::declare_class_element("PSMoveController.pos", &MyPSMoveController::pos);
	Kaba::declare_class_element("PSMoveController.acc", &MyPSMoveController::acc);
	Kaba::declare_class_element("PSMoveController.rot", &MyPSMoveController::rot);
	Kaba::declare_class_element("PSMoveController.ang", &MyPSMoveController::ang);
	Kaba::declare_class_element("PSMoveController.dang", &MyPSMoveController::dang);
	Kaba::declare_class_element("PSMoveController.trigger", &MyPSMoveController::trigger);
	Kaba::declare_class_element("PSMoveController.rumble", &MyPSMoveController::rumble);
	Kaba::declare_class_element("PSMoveController.button", &MyPSMoveController::button);
	Kaba::declare_class_element("PSMoveController.color", &MyPSMoveController::col);
	Kaba::link_external("psmove_init_tracker", (void*)&_psmove_init_tracker);
	Kaba::link_external("psmove_kill_tracker", (void*)&_psmove_kill_tracker);
	Kaba::link_external("psmove_update_tracker", (void*)&_psmove_update_tracker);
#endif


	Kaba::link_external("edward", &GlobalMainWin);
	Kaba::link_external("ed", &ed);
	Kaba::link_external("data_model", &mode_model->data);
	Kaba::link_external("data_world", &mode_world->data);

	Kaba::declare_class_element("Edward.cur_mode", &Edward::cur_mode);

	Kaba::declare_class_element("Mode.name", &ModeBase::name);
	Kaba::declare_class_element("Mode.multi_view", &ModeBase::multi_view);

	Kaba::declare_class_size("Observable", sizeof(Observable));
	//Kaba::declare_class_element("Observable.observable_name", offsetof(Observable, observable_name));

	Kaba::declare_class_size("Data", sizeof(Data));
	Kaba::declare_class_element("Data.filename", &Data::filename);
	Kaba::declare_class_element("Data.file_time", &Data::file_time);
	Kaba::declare_class_element("Data.binary_file_format", &Data::binary_file_format);
	Kaba::declare_class_element("Data.type", &Data::type);
	Kaba::link_external_class_func("Data.BeginActionGroup", &Data::begin_action_group);
	Kaba::link_external_class_func("Data.EndActionGroup", &Data::end_action_group);

	Kaba::declare_class_size("MultiViewSingleData", sizeof(MultiView::SingleData));
	Kaba::declare_class_element("MultiViewSingleData.pos", &MultiView::SingleData::pos);
	Kaba::declare_class_element("MultiViewSingleData.view_stage", &MultiView::SingleData::view_stage);
	Kaba::declare_class_element("MultiViewSingleData.is_selected", &MultiView::SingleData::is_selected);
	Kaba::declare_class_element("MultiViewSingleData.m_delta", &MultiView::SingleData::m_delta);
	Kaba::declare_class_element("MultiViewSingleData.m_old", &MultiView::SingleData::m_old);
	Kaba::declare_class_element("MultiViewSingleData.is_special", &MultiView::SingleData::is_special);


	Kaba::declare_class_element("MultiView.action_con", &MultiView::MultiView::action_con);
	Kaba::declare_class_element("MultiView.active_win", &MultiView::MultiView::active_win);
	Kaba::declare_class_element("MultiView.mouse_win", &MultiView::MultiView::mouse_win);
	Kaba::declare_class_element("MultiView.cam", &MultiView::MultiView::cam);
	Kaba::link_external_class_func("MultiView.cam_move", &MultiView::MultiView::cam_move);
	Kaba::link_external_class_func("MultiView.cam_rotate", &MultiView::MultiView::cam_rotate);
	Kaba::link_external_class_func("MultiView.cam_zoom", &MultiView::MultiView::cam_zoom);

	Kaba::declare_class_element("MultiViewWindow.local_ang", &MultiView::Window::local_ang);

	Kaba::link_external_class_func("ActionController.start_action", &MultiView::ActionController::start_action);
	Kaba::link_external_class_func("ActionController.update_action", &MultiView::ActionController::update_action);
	Kaba::link_external_class_func("ActionController.update_param", &MultiView::ActionController::update_param);
	Kaba::link_external_class_func("ActionController.end_action", &MultiView::ActionController::end_action);
	Kaba::declare_class_element("ActionController.pos", &MultiView::ActionController::pos);

	// model

	Kaba::declare_class_size("ModelEffect", sizeof(ModelEffect));
	Kaba::declare_class_element("ModelEffect.kind", &ModelEffect::type);
	Kaba::declare_class_element("ModelEffect.surface", &ModelEffect::surface);
	Kaba::declare_class_element("ModelEffect.vertex", &ModelEffect::vertex);
	Kaba::declare_class_element("ModelEffect.size", &ModelEffect::size);
	Kaba::declare_class_element("ModelEffect.speed", &ModelEffect::speed);
	Kaba::declare_class_element("ModelEffect.intensity", &ModelEffect::intensity);
	Kaba::declare_class_element("ModelEffect.colors", &ModelEffect::colors);
	Kaba::declare_class_element("ModelEffect.inv_quad", &ModelEffect::inv_quad);
	Kaba::declare_class_element("ModelEffect.file", &ModelEffect::file);

	Kaba::declare_class_size("ModelVertex", sizeof(ModelVertex));
	Kaba::declare_class_element("ModelVertex.normal_mode", &ModelVertex::normal_mode);
	Kaba::declare_class_element("ModelVertex.bone_index", &ModelVertex::bone_index);
	Kaba::declare_class_element("ModelVertex.normal_dirty", &ModelVertex::normal_dirty);
	Kaba::declare_class_element("ModelVertex.ref_count", &ModelVertex::ref_count);
	Kaba::declare_class_element("ModelVertex.surface", &ModelVertex::surface);

	Kaba::declare_class_size("ModelPolygonSide", sizeof(ModelPolygonSide));
	Kaba::declare_class_element("ModelPolygonSide.vertex", &ModelPolygonSide::vertex);
	Kaba::declare_class_element("ModelPolygonSide.edge", &ModelPolygonSide::edge);
	Kaba::declare_class_element("ModelPolygonSide.edge_direction", &ModelPolygonSide::edge_direction);
	Kaba::declare_class_element("ModelPolygonSide.skin_vertex", &ModelPolygonSide::skin_vertex);
	Kaba::declare_class_element("ModelPolygonSide.normal_index", &ModelPolygonSide::normal_index);
	Kaba::declare_class_element("ModelPolygonSide.normal", &ModelPolygonSide::normal);
	Kaba::declare_class_element("ModelPolygonSide.triangulation", &ModelPolygonSide::triangulation);

	Kaba::declare_class_size("ModelPolygon", sizeof(ModelPolygon));
	Kaba::declare_class_element("ModelPolygon.side", &ModelPolygon::side);
	Kaba::declare_class_element("ModelPolygon.temp_normal", &ModelPolygon::temp_normal);
	Kaba::declare_class_element("ModelPolygon.normal_dirty", &ModelPolygon::normal_dirty);
	Kaba::declare_class_element("ModelPolygon.triangulation_dirty", &ModelPolygon::triangulation_dirty);
	Kaba::declare_class_element("ModelPolygon.material", &ModelPolygon::material);

	Kaba::declare_class_size("ModelSurface", sizeof(ModelSurface));
	Kaba::declare_class_element("ModelSurface.polygon", &ModelSurface::polygon);
	Kaba::declare_class_element("ModelSurface.edge", &ModelSurface::edge);
	Kaba::declare_class_element("ModelSurface.vertex", &ModelSurface::vertex);
	Kaba::declare_class_element("ModelSurface.is_closed", &ModelSurface::is_closed);
	Kaba::declare_class_element("ModelSurface.is_physical", &ModelSurface::is_physical);
	Kaba::declare_class_element("ModelSurface.is_visible", &ModelSurface::is_visible);
	Kaba::declare_class_element("ModelSurface.model", &ModelSurface::model);

	Kaba::declare_class_size("ModelBone", sizeof(ModelBone));
	Kaba::declare_class_element("ModelBone.parent", &ModelBone::parent);
	Kaba::declare_class_element("ModelBone.model_file", &ModelBone::model_file);
	Kaba::declare_class_element("ModelBone.model", &ModelBone::model);
	Kaba::declare_class_element("ModelBone.const_pos", &ModelBone::const_pos);
	Kaba::declare_class_element("ModelBone.matrix", &ModelBone::_matrix);

	Kaba::declare_class_size("ModelMove", sizeof(ModelMove));
	Kaba::declare_class_element("ModelMove.name", &ModelMove::name);
	Kaba::declare_class_element("ModelMove.type", &ModelMove::type);
	Kaba::declare_class_element("ModelMove.FramesPerSecConst", &ModelMove::frames_per_sec_const);
	Kaba::declare_class_element("ModelMove.FramesPerSecFactor", &ModelMove::frames_per_sec_factor);
	Kaba::declare_class_element("ModelMove.frame", &ModelMove::frame);

	Kaba::declare_class_size("ModelFrame", sizeof(ModelFrame));
	Kaba::declare_class_element("ModelFrame.skel_ang", &ModelFrame::skel_ang);
	Kaba::declare_class_element("ModelFrame.skel_dpos", &ModelFrame::skel_dpos);
	Kaba::declare_class_element("ModelFrame.dpos", &ModelFrame::skin);
	Kaba::declare_class_element("ModelFrame.vertex_dpos", &ModelFrame::vertex_dpos);

	Kaba::declare_class_size("DataModel", sizeof(DataModel));
	Kaba::declare_class_element("DataModel.bone", &DataModel::bone);
	Kaba::declare_class_element("DataModel.move", &DataModel::move);
	//Kaba::declare_class_element("DataModel.move", &DataModel::move);
	//Kaba::declare_class_element("DataModel.CurrentMove", &DataModel::CurrentMove);
	//Kaba::declare_class_element("DataModel.CurrentFrame", &DataModel::CurrentFrame);
	Kaba::declare_class_element("DataModel.vertex", &DataModel::vertex);
	Kaba::declare_class_element("DataModel.surface", &DataModel::surface);
	Kaba::declare_class_element("DataModel.ball", &DataModel::ball);
	Kaba::declare_class_element("DataModel.poly", &DataModel::poly);
	Kaba::declare_class_element("DataModel.material", &DataModel::material);
	Kaba::declare_class_element("DataModel.fx", &DataModel::fx);
	Kaba::declare_class_element("DataModel.meta_data", &DataModel::meta_data);
	Kaba::declare_class_element("DataModel.min", &DataModel::_min);
	Kaba::declare_class_element("DataModel.max", &DataModel::_max);
	//Kaba::declare_class_element("DataModel.CurrentMaterial", &DataModel::CurrentMaterial);
	//Kaba::declare_class_element("DataModel.CurrentTextureLevel", &DataModel::CurrentTextureLevel);
	//Kaba::declare_class_element("DataModel.SkinVertex", &DataModel::SkinVertex);
	//Kaba::declare_class_element("DataModel.SkinVertMat", &DataModel::SkinVertMat);
	/*Kaba::declare_class_element("DataModel.SkinVertTL", &DataModel::SkinVertTL);
	Kaba::declare_class_element("DataModel.Playing", &DataModel::Playing);
	Kaba::declare_class_element("DataModel.PlayLoop", &DataModel::PlayLoop);
	Kaba::declare_class_element("DataModel.TimeScale", &DataModel::TimeScale);
	Kaba::declare_class_element("DataModel.TimeParam", &DataModel::TimeParam);
	Kaba::declare_class_element("DataModel.SimFrame", &DataModel::SimFrame);*/
	Kaba::link_external_class_func("DataModel.ClearSelection", &DataModel::clearSelection);
	Kaba::link_external_class_func("DataModel.InvertSelection", &DataModel::invertSelection);
	Kaba::link_external_class_func("DataModel.SelectionFromVertices", &DataModel::selectionFromVertices);
	Kaba::link_external_class_func("DataModel.SelectionFromPolygons", &DataModel::selectionFromPolygons);
	Kaba::link_external_class_func("DataModel.SelectionFromSurfaces", &DataModel::selectionFromSurfaces);
	Kaba::link_external_class_func("DataModel.SelectOnlySurface", &DataModel::selectOnlySurface);
	Kaba::link_external_class_func("DataModel.AddVertex", &DataModel::addVertex);
	Kaba::link_external_class_func("DataModel.AddTriangle", &DataModel::addTriangle);
	Kaba::link_external_class_func("DataModel.AddPolygon", &DataModel::addPolygon);
	Kaba::link_external_class_func("DataModel.DeleteSelection", &DataModel::deleteSelection);
	Kaba::link_external_class_func("DataModel.SubtractSelection", &DataModel::subtractSelection);
	Kaba::link_external_class_func("DataModel.ConvertSelectionToTriangles", &DataModel::convertSelectionToTriangles);
	Kaba::link_external_class_func("DataModel.TriangulateSelectedVertices", &DataModel::triangulateSelectedVertices);
	Kaba::link_external_class_func("DataModel.BevelSelectedVertices", &DataModel::bevelSelectedEdges);
	Kaba::link_external_class_func("DataModel.ExtrudeSelectedPolygons", &DataModel::extrudeSelectedPolygons);
	Kaba::link_external_class_func("DataModel.PasteGeometry", &DataModel::pasteGeometry);
	Kaba::link_external_class_func("DataModel.add_animation", &DataModel::addAnimation);
	Kaba::link_external_class_func("DataModel.duplicate_animation", &DataModel::duplicateAnimation);
	Kaba::link_external_class_func("DataModel.delete_animation", &DataModel::deleteAnimation);
	Kaba::link_external_class_func("DataModel.animation_set_data", &DataModel::setAnimationData);
	Kaba::link_external_class_func("DataModel.animation_add_frame", &DataModel::animationAddFrame);
	Kaba::link_external_class_func("DataModel.animation_delete_frame", &DataModel::animationDeleteFrame);
	Kaba::link_external_class_func("DataModel.animation_set_frame_duration", &DataModel::animationSetFrameDuration);
	Kaba::link_external_class_func("DataModel.animation_set_bone", &DataModel::animationSetBone);


	Kaba::declare_class_size("Geometry", sizeof(Geometry));
	Kaba::link_external_class_func("GeometryBall.__init__", &GeometryBall::__init__);
	Kaba::link_external_class_func("GeometryCube.__init__", &GeometryCube::__init__);
	Kaba::link_external_class_func("GeometrySphere.__init__", &GeometrySphere::__init__);
	Kaba::link_external_class_func("GeometryCylinder.__init__", &GeometryCylinder::__init__);
	Kaba::link_external_class_func("GeometryCylinderComplex.__init__", &GeometryCylinder::__init2__);
	Kaba::link_external_class_func("GeometryTorus.__init__", &GeometryTorus::__init__);
	Kaba::link_external_class_func("GeometryTorusKnot.__init__", &GeometryTorusKnot::__init__);
	Kaba::link_external_class_func("GeometryPlane.__init__", &GeometryPlane::__init__);
	Kaba::link_external_class_func("GeometryPlatonic.__init__", &GeometryPlatonic::__init__);
	Kaba::link_external_class_func("GeometryTeapot.__init__", &GeometryTeapot::__init__);
	Kaba::link_external("GeometrySubtract", (void*)&GeometrySubtract);
	Kaba::link_external("GeometryAnd", (void*)&GeometryAnd);

	// world

	Kaba::declare_class_size("DataWorld", sizeof(DataWorld));
	Kaba::declare_class_element("DataWorld.terrains", &DataWorld::Terrains);
	Kaba::declare_class_element("DataWorld.objects", &DataWorld::Objects);
	Kaba::declare_class_element("DataWorld.ego_index", &DataWorld::EgoIndex);
	Kaba::link_external_class_func("DataWorld.add_object", &DataWorld::AddObject);
	Kaba::link_external_class_func("DataWorld.add_terrain", &DataWorld::AddTerrain);
	Kaba::link_external_class_func("DataWorld.add_new_terrain", &DataWorld::AddNewTerrain);
}


