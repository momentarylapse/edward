/*
 * Mode.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODE_H_
#define MODE_H_

#include "../lib/hui/hui.h"
#include "../lib/math/math.h"
#include "../Data/Data.h"
#include "../Stuff/Observer.h"

namespace MultiView{
	class MultiView;
	class Window;
};

class ModeBase : public Observer
{
public:
	ModeBase(const string &name, ModeBase *parent, MultiView::MultiView *multi_view, const string &menu_id);
	virtual ~ModeBase();

	// Start/End: (once) entering this mode or a sub mode
	virtual void onStart(){};
	virtual void onEnd(){};
	// Enter/Leave: exactly this mode
	virtual void onEnter(){};
	virtual void onLeave(){};

	// events to be handled by derived modes
	virtual void onMouseMove(){};
	virtual void onMouseWheel(){};
	virtual void onMouseEnter(){};
	virtual void onMouseLeave(){};
	virtual void onLeftButtonDown(){};
	virtual void onLeftButtonUp(){};
	virtual void onMiddleButtonDown(){};
	virtual void onMiddleButtonUp(){};
	virtual void onRightButtonDown(){};
	virtual void onRightButtonUp(){};
	virtual void onKeyDown(){};
	virtual void onKeyUp(){};
	virtual void onCommand(const string &id){};
	virtual void onUpdateMenu(){};

	virtual void onDraw(){};
	virtual void onDrawWin(MultiView::Window *win){};

	virtual bool optimizeView(){ return false; };
	virtual void optimizeViewRecursice();

	// send events to multi_view first, then call derived event handlers
	//   (to be called by edward)
	virtual void onMouseMoveRecursive(bool multi_view_handled = false);
	virtual void onMouseWheelRecursive(bool multi_view_handled = false);
	virtual void onMouseEnterRecursive(bool multi_view_handled = false);
	virtual void onMouseLeaveRecursive(bool multi_view_handled = false);
	virtual void onLeftButtonDownRecursive(bool multi_view_handled = false);
	virtual void onLeftButtonUpRecursive(bool multi_view_handled = false);
	virtual void onMiddleButtonDownRecursive(bool multi_view_handled = false);
	virtual void onMiddleButtonUpRecursive(bool multi_view_handled = false);
	virtual void onRightButtonDownRecursive(bool multi_view_handled = false);
	virtual void onRightButtonUpRecursive(bool multi_view_handled = false);
	virtual void onKeyDownRecursive(bool multi_view_handled = false);
	virtual void onKeyUpRecursive(bool multi_view_handled = false);
	virtual void onCommandRecursive(const string &id, bool multi_view_handled = false);
	virtual void onUpdateMenuRecursive(bool multi_view_handled = false);

	virtual void onDrawRecursive(bool multi_view_handled = false);
	virtual void onDrawWinRecursive(MultiView::Window *win);

	ModeBase *getRoot();
	bool isAncestorOf(ModeBase *m);
	ModeBase *getNextChildTo(ModeBase *target);
	bool equalRoots(ModeBase *m);
	virtual Data *getData() = 0;


	virtual void _new();
	virtual bool open();
	virtual bool save();
	virtual bool saveAs();

	string name;

	ModeBase *parent;
	MultiView::MultiView *multi_view;

	string menu_id;
};

template<class T>
class Mode : public ModeBase
{
public:
	Mode(const string &name, ModeBase *parent, Data *_data, MultiView::MultiView *multi_view, const string &menu) :
		ModeBase(name, parent, multi_view, menu)
	{
		data = (T*)_data;
	}
	Mode(const string &name, ModeBase *parent, MultiView::MultiView *multi_view, const string &menu) :
		ModeBase(name, parent, multi_view, menu)
	{
		data = (T*)parent->getData();
	}
	virtual ~Mode(){}
	T *data;
	virtual Data *getData()
	{	return (Data*)data;	}
};

#endif /* MODE_H_ */
