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
#include "../Stuff/Observer.h"

class Mode : public Observer
{
public:
	Mode();
	virtual ~Mode();

	virtual void Start() = 0;
	virtual void End() = 0;

	// events to be handled by derived modes
	virtual void OnMouseMove(){};
	virtual void OnLeftButtonDown(){};
	virtual void OnLeftButtonUp(){};
	virtual void OnMiddleButtonDown(){};
	virtual void OnMiddleButtonUp(){};
	virtual void OnRightButtonDown(){};
	virtual void OnRightButtonUp(){};
	virtual void OnKeyDown(){};
	virtual void OnKeyUp(){};
	virtual void OnCommand(const string &id){};
	virtual void OnUpdateMenu(){};

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
	void OnPreUpdateMenu();
	Mode *GetRootMode();

	virtual void Draw(){};
	virtual void DrawWin(int win, irect dest){};

	string name;

	Mode *parent;
	MultiView *multi_view;

	CHuiMenu *menu;
};

#endif /* MODE_H_ */
