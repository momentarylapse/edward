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

class ActionMultiView;
class Data;


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
struct MultiViewSingleData
{
	int view_stage;
	bool is_selected, m_delta, m_old, is_special;
	vector pos;
};

struct MultiViewView
{
	int type;
	matrix mat;
	irect dest;
	irect name_dest;
};


typedef bool t_is_mouse_over_func(int index, void *user_data, int win, vector &tp);
typedef bool t_is_in_rect_func(int index, void *user_data, int win, irect *r);


struct MultiViewData{
	int Num, Type;
	bool MVSelectable, Drawable, Movable, Indexable;
	void *data, *user_data;
	int DataSingleSize;
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

// TODO refactor me!!!
class MultiView
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
	void OnLeftButtonDown();
	void OnLeftButtonUp();
	void OnMiddleButtonDown();
	void OnMiddleButtonUp();
	void OnRightButtonDown();
	void OnRightButtonUp();
	void OnKeyDown();
	void OnKeyUp();
	void OnCommand(const string &id);

	void Draw();
	void DrawWin(int win, irect dest);
	void DrawMousePos();
	void DrawGrid(int win, irect dest);
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
	void ResetData(Data *_data);
	void ViewStagePush();
	void ViewStagePop();
	void InvertSelection();

	void HoldCursor(bool holding);
	void StartRect();
	void EndRect();


	void GetMouseOver();
	void UnselectAll();
	void GetSelected();
	void SelectAllInRectangle();


	vector VecProject(vector p, int win);
	vector VecUnProject(vector p, int win);
	vector VecUnProject2(vector p, vector o, int win);
	vector GetDirection(int win);
	vector GetDirectionUp(int win);
	vector GetDirectionRight(int win);
	vector GetCursor3d();

	bool mode3d;

	vector pos, ang;
	float radius;
	float zoom;
	bool whole_window;
	bool ignore_radius;

	bool wire_mode;
	bool grid_enabled;
	bool light_enabled;

	int light;

	MultiViewView view[5];
	int cur_view, cur_view_rect;
	int view_stage;

	MultiViewMouseAction action[3];
	int active_mouse_action;
	ActionMultiView *cur_action;
	Data *_data_;
	vector mouse_action_param, mouse_action_pos0;
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

	int ViewMoving;

	int mx, my, mouse_win;
	int vx, vy;
	bool HoldingCursor;
	bool MVRect,MVRectable;
	int RectX,RectY,RectWin;

	bool MultiViewSelectionChanged;


	int MouseMovedSinceClick;
	bool Moved;
	int MovingWin;
	vector MovingStart, MovingDP;
	bool MultiViewEditing;

	CHuiMenu *menu;
};

#endif /* MULTIVIEW_H_ */
