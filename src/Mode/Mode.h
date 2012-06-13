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

	virtual void OnStart() = 0;
	virtual void OnEnd() = 0;

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

	virtual void OnDraw(){};
	virtual void OnDrawWin(int win, irect dest){};

	// send events to multi_view first, then call derived event handlers
	//   (to be called by edward)
	virtual void OnMouseMoveRecursive();
	virtual void OnLeftButtonDownRecursive();
	virtual void OnLeftButtonUpRecursive();
	virtual void OnMiddleButtonDownRecursive();
	virtual void OnMiddleButtonUpRecursive();
	virtual void OnRightButtonDownRecursive();
	virtual void OnRightButtonUpRecursive();
	virtual void OnKeyDownRecursive();
	virtual void OnKeyUpRecursive();
	virtual void OnCommandRecursive(const string &id);
	virtual void OnUpdateMenuRecursive();

	virtual void OnDrawRecursive();
	//virtual void OnDrawWinRecursive(int win, irect dest);

	Mode *GetRootMode();
	virtual Data *GetData(){	return NULL;	}


	virtual void New();
	virtual bool Open();
	virtual bool Save();
	virtual bool SaveAs();

	string name;

	Mode *parent;
	MultiView *multi_view;

	CHuiMenu *menu;
};

#endif /* MODE_H_ */
