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

class ModeBase : public Observer
{
public:
	ModeBase(const string &name, ModeBase *parent, MultiView *multi_view, const string &menu);
	virtual ~ModeBase();

	// Start/End: (once) entering this mode or a sub mode
	virtual void OnStart(){};
	virtual void OnEnd(){};
	// Enter/Leave: exactly this mode
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
	virtual void OnDrawWin(MultiViewWindow *win){};

	virtual bool OptimizeView(){ return false; };
	virtual void OptimizeViewRecursice();

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
	virtual void OnDrawWinRecursive(MultiViewWindow *win);

	ModeBase *GetRoot();
	bool IsAncestorOf(ModeBase *m);
	ModeBase *GetNextChildTo(ModeBase *target);
	bool EqualRoots(ModeBase *m);
	virtual Data *GetData() = 0;


	virtual void New();
	virtual bool Open();
	virtual bool Save();
	virtual bool SaveAs();

	string name;

	ModeBase *parent;
	MultiView *multi_view;

	CHuiMenu *menu;
};

template<class T>
class Mode : public ModeBase
{
public:
	Mode(const string &name, ModeBase *parent, Data *_data, MultiView *multi_view, const string &menu) :
		ModeBase(name, parent, multi_view, menu)
	{
		data = (T*)_data;
	}
	Mode(const string &name, ModeBase *parent, MultiView *multi_view, const string &menu) :
		ModeBase(name, parent, multi_view, menu)
	{
		data = (T*)parent->GetData();
	}
	virtual ~Mode(){}
	T *data;
	virtual Data *GetData()
	{	return data;	}
};

#endif /* MODE_H_ */
