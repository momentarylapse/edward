/*
 * MultiView.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MULTIVIEW_H_
#define MULTIVIEW_H_

#include "lib/file/file.h"
#include "lib/nix/nix.h"
#include "Action/ActionMultiView.h"
#include "Data/Data.h"
#include "MultiView.h"
#include "Stuff/Observable.h"

class MultiView;
class ActionMultiView;
class Data;
class Observable;
class ModelGeometry;


enum
{
	ViewRight,
	ViewLeft,
	ViewFront,
	ViewBack,
	ViewTop,
	ViewBottom,
	ViewPerspective,
	View2D,
	ViewIsometric
};

// "des Pudels Kern", don't change!!!!!!!
class MultiViewSingleData
{
public:
	MultiViewSingleData();
	int view_stage;
	bool is_selected, m_delta, m_old, is_special;
	vector pos;
};

struct MultiViewCamera
{
	vector pos, ang;
	float zoom, radius;
	bool ignore_radius;
};

struct MultiViewWindow
{
	int type;
	matrix mat;
	rect dest;
	rect name_dest;
	vector ang;
	matrix projection;
	MultiView *multi_view;
	MultiViewCamera *cam;

	void Draw();
	void DrawGrid();
	vector Project(const vector &p);
	vector Unproject(const vector &p);
	vector Unproject(const vector &p, const vector &o);
	vector GetDirection();
	vector GetDirectionUp();
	vector GetDirectionRight();
	void GetMovingFrame(vector &dir, vector &up, vector &right);
};


typedef bool t_is_mouse_over_func(int index, void *user_data, MultiViewWindow *win, vector &tp);
typedef bool t_is_in_rect_func(int index, void *user_data, MultiViewWindow *win, rect *r);


struct MultiViewData{
	int Type;
	DynamicArray *data;
	bool MVSelectable, Drawable, Movable, Indexable;
	void *user_data;
	//int DataSingleSize;
	t_is_mouse_over_func *IsMouseOver;
	t_is_in_rect_func *IsInRect;
};

struct MultiViewMouseAction
{
	string name;
	int mode;
	bool active;

	void reset()
	{
		name = "";
		mode = 0;
		active = false;
	}
};

struct MultiViewActionController
{
	bool show, captured;
	vector pos;
	int mode;
	Array<ModelGeometry*> geo;
	MultiView *multi_view;
	void reset();
	void Draw(MultiViewWindow *win);
	void Disable();
	void Enable();
	void Update();
	bool Click();
};

// TODO refactor me!!!
class MultiView : public Observable
{
public:
	MultiView(bool _mode3d);
	virtual ~MultiView();

	// multiview mask (data)
	static const int FlagNone = 0;
	static const int FlagSelect= 1;
	static const int FlagDraw = 2;
	static const int FlagIndex = 4;
	static const int FlagMove = 8;

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
	void SetViewStage(int *view_stage, bool allow_handle);
	void SetData(int type, const DynamicArray &a, void *user_data, int mode, t_is_mouse_over_func *is_mouse_over_func, t_is_in_rect_func *is_in_rect_func);
	//void DoMultiViewStuff(bool rect_able);
	void Reset();
	void ResetView();
	void SetViewBox(const vector &min, const vector &max);
	void ResetData(Data *_data);
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

	vector GetCursor3d();
	vector GetCursor3d(const vector &depth_reference);

	bool mode3d;

	MultiViewCamera cam;

	bool wire_mode;
	bool grid_enabled;
	bool light_enabled;

	int light;

	MultiViewWindow win[4];
	MultiViewWindow *cur_projection_win;
	MultiViewWindow *active_win;
	bool whole_window;
	int view_stage;

	bool allow_mouse_actions;
	MultiViewMouseAction action[3];
	int active_mouse_action;
	ActionMultiView *cur_action;
	MultiViewActionController action_con;
	Data *_data_;
	vector mouse_action_param, mouse_action_pos0;
	enum{
		ActionNone,
		ActionSelect,
		ActionMove,
		ActionRotate,
		ActionRotate2d,
		ActionScale,
		ActionScale2d,
		ActionOnce
	};
	void ResetMouseAction();
	void SetMouseAction(int button, const string &name, int mode);
	void MouseActionStart(int button);
	void MouseActionEnd(bool set);
	void MouseActionUpdate();

	Array<MultiViewData> data;
	bool AllowViewStage, AllowViewStageHandling;
	int MouseOver,MouseOverType,MouseOverSet,Selected,SelectedType,SelectedSet;
	vector MouseOverTP,SelectedTP,MovingDPos,RFPos,LFPos;
	bool EditingStart, EditingEnd, DataChanged, Changed;

	MultiViewWindow *mouse_win;
	vector m, v;
	bool HoldingCursor;
	float HoldingX, HoldingY;
	bool MVRect, allow_rect;
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
