/*
 * Mode.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Mode.h"
#include "../MultiView/MultiView.h"
#include "../MultiView/MultiViewImpl.h"

ModeBase::ModeBase(const string &_name, ModeBase *_parent, MultiView::MultiView *_multi_view, const string &_menu) :
	Observer(_name)
{
	parent = _parent;
	name = _name;
	multi_view = _multi_view;
	menu_id = _menu;
	/*if ((!menu) && (parent))
		menu = parent->menu;*/
}

ModeBase::~ModeBase()
{
}

void ModeBase::onCommandRecursive(const string & id, bool multi_view_handled)
{
	if ((multi_view) && (!multi_view_handled)){
		((MultiView::MultiViewImpl*)multi_view)->onCommand(id);
		multi_view_handled = true;
	}
	if (parent)
		parent->onCommandRecursive(id, multi_view_handled);
	onCommand(id);
}

#define CREATE_EVENT_HANDLER(NAME_REC, NAME)	\
void ModeBase::NAME_REC(bool multi_view_handled) \
{ \
	if ((multi_view) && (!multi_view_handled)){ \
		((MultiView::MultiViewImpl*)multi_view)->NAME(); \
		multi_view_handled = true; \
	} \
	if (parent) \
		parent->NAME_REC(multi_view_handled); \
	NAME(); \
}

CREATE_EVENT_HANDLER(onLeftButtonDownRecursive, onLeftButtonDown)
CREATE_EVENT_HANDLER(onLeftButtonUpRecursive, onLeftButtonUp)
CREATE_EVENT_HANDLER(onMiddleButtonDownRecursive, onMiddleButtonDown)
CREATE_EVENT_HANDLER(onMiddleButtonUpRecursive, onMiddleButtonUp)
CREATE_EVENT_HANDLER(onRightButtonDownRecursive, onRightButtonDown)
CREATE_EVENT_HANDLER(onRightButtonUpRecursive, onRightButtonUp)
CREATE_EVENT_HANDLER(onMouseMoveRecursive, onMouseMove)
CREATE_EVENT_HANDLER(onMouseWheelRecursive, onMouseWheel)
CREATE_EVENT_HANDLER(onMouseEnterRecursive, onMouseEnter)
CREATE_EVENT_HANDLER(onMouseLeaveRecursive, onMouseLeave)
CREATE_EVENT_HANDLER(onKeyDownRecursive, onKeyDown)
CREATE_EVENT_HANDLER(onKeyUpRecursive, onKeyUp)
CREATE_EVENT_HANDLER(onDrawRecursive, onDraw)

void ModeBase::onUpdateMenuRecursive(bool multi_view_handled)
{
	if (parent)
		parent->onUpdateMenuRecursive();
	onUpdateMenu();
}

void ModeBase::onDrawWinRecursive(MultiView::Window *win)
{
	if (parent)
		parent->onDrawWinRecursive(win);
	onDrawWin(win);
}

void ModeBase::optimizeViewRecursice()
{
	if (optimizeView())
		return;
	if (parent)
		parent->optimizeViewRecursice();
}



ModeBase *ModeBase::getRoot()
{
	if (parent)
		return parent->getRoot();
	return this;
}

bool ModeBase::isAncestorOf(ModeBase *m)
{
	if (m == this)
		return true;
	if (m->parent)
		return isAncestorOf(m->parent);
	return false;
}


ModeBase *ModeBase::getNextChildTo(ModeBase *target)
{
	while(target){
		if (this == target->parent)
			return target;
		target = target->parent;
	}
	return NULL;
}


bool ModeBase::equalRoots(ModeBase *m)
{
	if (!m)
		return false;
	return getRoot() == m->getRoot();
}

void ModeBase::_new()
{
	if (parent)
		parent->_new();
}

bool ModeBase::save()
{
	if (parent)
		return parent->save();
	return true;
}



bool ModeBase::saveAs()
{
	if (parent)
		return parent->saveAs();
	return true;
}



bool ModeBase::open()
{
	if (parent)
		return parent->open();
	return true;
}





