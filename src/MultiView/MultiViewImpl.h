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


struct DataSet
{
	int Type;
	DynamicArray *data;
	bool MVSelectable, Drawable, Movable, Indexable;
	void *user_data;
};



extern color ColorBackGround3D;
extern color ColorBackGround2D;
extern color ColorGrid;
extern color ColorText;
extern color ColorWindowType;
extern color ColorPoint;
extern color ColorPointSelected;
extern color ColorPointSpecial;
extern color ColorWindowSeparator;
extern color ColorSelectionRect;
extern color ColorSelectionRectBoundary;

extern int PointRadius;
extern int PointRadiusMouseOver;


class MultiViewImpl : public MultiView
{
public:
	MultiViewImpl(bool _mode3d);
	virtual ~MultiViewImpl();

	void OnMouseMove();
	void OnMouseWheel();
	void OnLeftButtonDown();
	void OnLeftButtonUp();
	void OnMiddleButtonDown();
	void OnMiddleButtonUp();
	void OnRightButtonDown();
	void OnRightButtonUp();
	void OnKeyDown();
	void OnKeyUp();
	void OnCommand(const string &id);

	void OnDraw();
	void DrawMousePos();
	void ToggleWholeWindow();
	void ToggleGrid();
	void ToggleLight();
	void ToggleWire();
	void DoZoom(float factor);
	void DoMove(const vector &dir);
	void SetMode(int mode);
//	void SetFunctions(mv_bc_func *start_edit_func, mv_bc_func *end_edit_func, mv_bc_func *edit_func);
	virtual void SetViewStage(int *view_stage, bool allow_handle);
	virtual void SetData(int type, const DynamicArray &a, void *user_data, int mode);
	virtual void Reset();
	virtual void ResetView();
	virtual void SetViewBox(const vector &min, const vector &max);
	virtual void ResetData(Data *_data);
	void ViewStagePush();
	void ViewStagePop();

	void SelectAll();
	void SelectNone();
	void InvertSelection();
	bool HasSelection();
	vector GetSelectionCenter();

	void HoldCursor(bool holding);
	void StartRect();
	void EndRect();
	void UpdateMouse();

	void GetMouseOver();
	void UnselectAll();
	enum{
		SelectSet,
		SelectAdd,
		SelectInvert
	};
	void GetSelected(int mode = SelectSet);
	void SelectAllInRectangle(int mode = SelectSet);

	virtual float GetGridD();
	string GetMVScaleByZoom(vector &v);

	vector virtual GetCursor3d();
	vector virtual GetCursor3d(const vector &depth_reference);


	Window *win[4];
	Window *cur_projection_win;

	ActionController *action_con;
	virtual void ResetMouseAction();
	virtual void SetMouseAction(const string &name, int mode);

	Array<DataSet> data;
	bool allow_view_stage, allow_view_stage_handling;
	vector MovingDPos,RFPos,LFPos;

	bool HoldingCursor;
	float HoldingX, HoldingY;

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
	bool ViewMoving;


	int MovingWin;
	vector MovingStart, MovingDP;

	HuiMenu *menu;

	struct Message3d
	{
		string str;
		vector pos;
	};
	Array<Message3d> message3d;
	virtual void AddMessage3d(const string &str, const vector &pos);
	virtual void ResetMessage3d();
};

};

#endif /* MULTIVIEWIMPL_H_ */
