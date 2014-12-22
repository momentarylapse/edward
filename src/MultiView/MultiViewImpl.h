/*
 * MultiViewImpl.h
 *
 *  Created on: 22.01.2014
 *      Author: michi
 */

#ifndef MULTIVIEWIMPL_H_
#define MULTIVIEWIMPL_H_

#include "MultiView.h"

namespace MultiView{

class ActionController;
class CameraController;


struct DataSet
{
	int type;
	DynamicArray *data;
	bool selectable, drawable, movable, indexable;
	void *user_data;
};



extern color ColorBackGround;
extern color ColorBackGroundSelected;
extern color ColorGrid;
extern color ColorText;
extern color ColorWindowType;
extern color ColorPoint;
extern color ColorPointSelected;
extern color ColorPointSpecial;
extern color ColorWindowSeparator;
extern color ColorSelectionRect;
extern color ColorSelectionRectBoundary;


class MultiViewImpl : public MultiView
{
public:
	MultiViewImpl(bool _mode3d);
	virtual ~MultiViewImpl();

	void onMouseMove();
	void onMouseWheel();
	void onMouseEnter();
	void onMouseLeave();
	void onLeftButtonDown();
	void onLeftButtonUp();
	void onMiddleButtonDown();
	void onMiddleButtonUp();
	void onRightButtonDown();
	void onRightButtonUp();
	void onKeyDown();
	void onKeyUp();
	void onCommand(const string &id);

	void onDraw();
	void drawMousePos();
	void toggleWholeWindow();
	void toggleGrid();
	void toggleLight();
	void toggleWire();
	void camZoom(float factor, bool mouse_rel);
	void camMove(const vector &dir);
	void camRotate(const vector &dir, bool cam_center);
	void setMode(int mode);
	virtual void clearData(Data *_data);
	virtual void addData(int type, const DynamicArray &a, void *user_data, int flags);
	virtual void SetViewStage(int *view_stage, bool allow_handle);
	virtual void reset();
	virtual void resetView();
	virtual void setViewBox(const vector &min, const vector &max);
	virtual void setAllowSelect(bool allow);
	virtual void setAllowAction(bool allow);
	void viewStagePush();
	void viewStagePop();

	void selectAll();
	void selectNone();
	void invertSelection();
	bool hasSelection();
	vector getSelectionCenter();

	void holdCursor(bool holding);
	void startRect();
	void endRect();
	void updateMouse();

	void getHover();
	void unselectAll();
	enum{
		SELECT_SET,
		SELECT_ADD,
		SELECT_INVERT
	};
	void getSelected(int mode = SELECT_SET);
	void selectAllInRectangle(int mode = SELECT_SET);
	bool hoverSelected();
	bool hasSelectableData();

	virtual float getGridD();
	string getScaleByZoom(vector &v);

	vector virtual getCursor3d();
	vector virtual getCursor3d(const vector &depth_reference);


	Window *win[4];
	Window *cur_projection_win;

	bool lbut, mbut, rbut;

	ActionController *action_con;
	virtual void resetMouseAction();
	virtual void setMouseAction(const string &name, int mode, bool locked);
	bool needActionController();
	CameraController *cam_con;

	Array<DataSet> data;
	bool allow_view_stage, allow_view_stage_handling;

	bool allow_infinite_scrolling;
	bool holding_cursor;
	float holding_x, holding_y;

	struct SelectionRect
	{
		bool active;
		int dist;
		vector pos0;
		void start_later(const vector &m);
		void end();
		rect get(const vector &m);
		void draw(const vector &m);
	};
	SelectionRect sel_rect;
	bool allow_select;
	bool view_moving;


	int moving_win;
	vector moving_start, moving_dp;

	HuiMenu *menu;

	struct Message3d
	{
		string str;
		vector pos;
	};
	Array<Message3d> message3d;
	virtual void addMessage3d(const string &str, const vector &pos);
	virtual void resetMessage3d();


	float SPEED_MOVE;
	float SPEED_ZOOM_KEY;
	float SPEED_ZOOM_WHEEL;

	int MIN_MOUSE_MOVE_TO_INTERACT;
	float MOUSE_ROTATION_SPEED;



	int POINT_RADIUS;
	int POINT_RADIUS_HOVER;
};

};

#endif /* MULTIVIEWIMPL_H_ */
