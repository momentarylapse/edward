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

void ModeBase::onCommandRecursive(const string & id)
{
	if (parent)
		parent->onCommandRecursive(id);
	onCommand(id);
}

void ModeBase::onCommandMeta(const string & id)
{
	if (multi_view)
		((MultiView::MultiViewImpl*)multi_view)->onCommand(id);
	onCommandRecursive(id);
}

#define CREATE_EVENT_HANDLER(NAME_META, NAME)	\
void ModeBase::NAME_META() \
{ \
	if (multi_view) \
		((MultiView::MultiViewImpl*)multi_view)->NAME(); \
	NAME(); \
}

CREATE_EVENT_HANDLER(onLeftButtonDownMeta, onLeftButtonDown)
CREATE_EVENT_HANDLER(onLeftButtonUpMeta, onLeftButtonUp)
CREATE_EVENT_HANDLER(onMiddleButtonDownMeta, onMiddleButtonDown)
CREATE_EVENT_HANDLER(onMiddleButtonUpMeta, onMiddleButtonUp)
CREATE_EVENT_HANDLER(onRightButtonDownMeta, onRightButtonDown)
CREATE_EVENT_HANDLER(onRightButtonUpMeta, onRightButtonUp)
CREATE_EVENT_HANDLER(onMouseMoveMeta, onMouseMove)
CREATE_EVENT_HANDLER(onMouseWheelMeta, onMouseWheel)
CREATE_EVENT_HANDLER(onMouseEnterMeta, onMouseEnter)
CREATE_EVENT_HANDLER(onMouseLeaveMeta, onMouseLeave)
CREATE_EVENT_HANDLER(onKeyDownMeta, onKeyDown)
CREATE_EVENT_HANDLER(onKeyUpMeta, onKeyUp)
CREATE_EVENT_HANDLER(onDrawMeta, onDraw)

void ModeBase::onUpdateMenuMeta()
{
	onUpdateMenuRecursive();
}

void ModeBase::onUpdateMenuRecursive()
{
	if (parent)
		parent->onUpdateMenuRecursive();
	onUpdateMenu();
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





