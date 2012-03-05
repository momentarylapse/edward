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
};

class MultiView
{
public:
	MultiView(bool _mode3d);
	virtual ~MultiView();

	void Reset();

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
	void DoZoom(float factor);
	void DoMove(const vector &dir);


	vector VecProject(vector p, int win);
	vector VecUnProject(vector p, int win);
	vector VecUnProject2(vector p, vector o, int win);
	vector GetDirection(int win);
	vector GetDirectionUp(int win);
	vector GetDirectionRight(int win);
	vector GetCursor3D();

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

	int mx, my, mouse_win;

	CHuiMenu *menu;
};

extern MultiView *mv3d;
extern MultiView *mv2d;

#endif /* MULTIVIEW_H_ */
