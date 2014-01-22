/*
 * MultiView.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MULTIVIEW_H_
#define MULTIVIEW_H_

#include "../lib/file/file.h"
#include "../lib/nix/nix.h"
#include "MultiViewInterface_.h"
#include "../Action/ActionMultiView.h"
#include "../Data/Data.h"

class MultiView;
class MultiViewWindow;
class ActionController;
class ActionMultiView;
class Data;
class Observable;
class Geometry;


struct MultiViewData{
	int Type;
	DynamicArray *data;
	bool MVSelectable, Drawable, Movable, Indexable;
	void *user_data;
	//int DataSingleSize;
	t_is_mouse_over_func *IsMouseOver;
	t_is_in_rect_func *IsInRect;
};


// TODO refactor me!!!
class MultiView : public MultiViewInterface
{
public:
	MultiView(bool _mode3d);
	virtual ~MultiView();

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
	virtual void SetData(int type, const DynamicArray &a, void *user_data, int mode, t_is_mouse_over_func *is_mouse_over_func, t_is_in_rect_func *is_in_rect_func);
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

	color ColorBackGround3D;
	color ColorBackGround2D;
	color ColorGrid;
	color ColorText;
	color ColorWindowType;
	color ColorPoint;
	color ColorPointSelected;
	color ColorPointSpecial;
	color ColorWindowSeparator;
	color ColorSelectionRect;
	color ColorSelectionRectBoundary;

	int PointRadius;
	int PointRadiusMouseOver;

	void GetMouseOver();
	void UnselectAll();
	enum{
		SelectSet,
		SelectAdd,
		SelectInvert
	};
	void GetSelected(int mode = SelectSet);
	void SelectAllInRectangle(int mode = SelectSet);

	float GetGridD();
	string GetMVScaleByZoom(vector &v);

	vector virtual GetCursor3d();
	vector virtual GetCursor3d(const vector &depth_reference);

	bool mode3d;


	int light;

	MultiViewWindow *win[4];
	MultiViewWindow *cur_projection_win;
	MultiViewWindow *active_win;
	bool whole_window;

	ActionController *action_con;
	Data *_data_;
	virtual void ResetMouseAction();
	virtual void SetMouseAction(const string &name, int mode);

	Array<MultiViewData> data;
	bool AllowViewStage, AllowViewStageHandling;
	vector MovingDPos,RFPos,LFPos;
//	bool EditingStart, EditingEnd, DataChanged, Changed;

	MultiViewWindow *mouse_win;
	bool HoldingCursor;
	float HoldingX, HoldingY;
	bool MVRect;
	float RectX, RectY;
	bool ViewMoving;

	bool MultiViewSelectionChanged;


	int MouseMovedSinceClick;
	bool Moved;
	int MovingWin;
	vector MovingStart, MovingDP;
	bool MultiViewEditing;

	HuiMenu *menu;

	struct Message3d
	{
		string str;
		vector pos;
	};
	Array<Message3d> message3d;
	void AddMessage3d(const string &str, const vector &pos);
	void ResetMessage3d();
};

#endif /* MULTIVIEW_H_ */
