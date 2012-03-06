/*
 * Mode.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODE_H_
#define MODE_H_

#include "../lib/hui/hui.h"
#include "../Data/Data.h"
#include "../MultiView.h"

class Mode
{
public:
	Mode();
	virtual ~Mode();

	virtual void Start() = 0;
	virtual void End() = 0;

	// events to be handled by derived modes
	virtual void OnMouseMove() = 0;
	virtual void OnLeftButtonDown() = 0;
	virtual void OnLeftButtonUp() = 0;
	virtual void OnMiddleButtonDown() = 0;
	virtual void OnMiddleButtonUp() = 0;
	virtual void OnRightButtonDown() = 0;
	virtual void OnRightButtonUp() = 0;
	virtual void OnKeyDown() = 0;
	virtual void OnKeyUp() = 0;
	virtual void OnCommand(const string &id) = 0;
	virtual void OnDataChange() = 0;

	// send events to multi_view first, then call derived event handlers
	//   (to be called by edward)
	void OnPreMouseMove();
	void OnPreLeftButtonDown();
	void OnPreLeftButtonUp();
	void OnPreMiddleButtonDown();
	void OnPreMiddleButtonUp();
	void OnPreRightButtonDown();
	void OnPreRightButtonUp();
	void OnPreKeyDown();
	void OnPreKeyUp();
	void OnPreCommand(const string &id);

	virtual void Draw() = 0;
	virtual void DrawWin(int win, irect dest) = 0;

	Mode *parent;
	MultiView *multi_view;

	CHuiMenu *menu;
};

#endif /* MODE_H_ */
