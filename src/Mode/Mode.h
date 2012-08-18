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
	Mode(const string &_name, Mode *_parent, Data *_data, MultiView *_multi_view, const string &_menu);
	virtual ~Mode();

	virtual void OnStart(){};
	virtual void OnEnd(){};
	virtual void OnEnter(){};
	virtual void OnLeave(){};

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
	virtual void OnMouseMoveRecursive(bool multi_view_handled = false);
	virtual void OnLeftButtonDownRecursive(bool multi_view_handled = false);
	virtual void OnLeftButtonUpRecursive(bool multi_view_handled = false);
	virtual void OnMiddleButtonDownRecursive(bool multi_view_handled = false);
	virtual void OnMiddleButtonUpRecursive(bool multi_view_handled = false);
	virtual void OnRightButtonDownRecursive(bool multi_view_handled = false);
	virtual void OnRightButtonUpRecursive(bool multi_view_handled = false);
	virtual void OnKeyDownRecursive(bool multi_view_handled = false);
	virtual void OnKeyUpRecursive(bool multi_view_handled = false);
	virtual void OnCommandRecursive(const string &id, bool multi_view_handled = false);
	virtual void OnUpdateMenuRecursive(bool multi_view_handled = false);

	virtual void OnDrawRecursive(bool multi_view_handled = false);
	virtual void OnDrawWinRecursive(int win, irect dest);

	Mode *GetRoot();
	bool IsAncestorOf(Mode *m);
	Mode *GetNextChildTo(Mode *target);
	bool EqualRoots(Mode *m);
	virtual Data *GetData(){	return data_generic;	}


	virtual void New();
	virtual bool Open();
	virtual bool Save();
	virtual bool SaveAs();

	string name;

	Mode *parent;
	MultiView *multi_view;
	Data *data_generic;

	CHuiMenu *menu;
};

#endif /* MODE_H_ */
