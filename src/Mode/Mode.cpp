/*
 * Mode.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Mode.h"
#include "../MultiView/MultiView.h"
#include "../MultiView/MultiViewImpl.h"

ModeBase::ModeBase(const string &_name, ModeBase *_parent, MultiView::MultiView *_multi_view, const string &_menu)
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

void ModeBase::OnCommandRecursive(const string & id, bool multi_view_handled)
{
	if ((multi_view) && (!multi_view_handled)){
		((MultiView::MultiViewImpl*)multi_view)->OnCommand(id);
		multi_view_handled = true;
	}
	if (parent)
		parent->OnCommandRecursive(id, multi_view_handled);
	OnCommand(id);
}

#define CREATE_EVENT_HANDLER(_name_rec_, _name_)	\
void ModeBase::_name_rec_(bool multi_view_handled) \
{ \
	if ((multi_view) && (!multi_view_handled)){ \
		((MultiView::MultiViewImpl*)multi_view)->_name_(); \
		multi_view_handled = true; \
	} \
	if (parent) \
		parent->_name_rec_(multi_view_handled); \
	_name_(); \
}

CREATE_EVENT_HANDLER(OnLeftButtonDownRecursive, OnLeftButtonDown)
CREATE_EVENT_HANDLER(OnLeftButtonUpRecursive, OnLeftButtonUp)
CREATE_EVENT_HANDLER(OnMiddleButtonDownRecursive, OnMiddleButtonDown)
CREATE_EVENT_HANDLER(OnMiddleButtonUpRecursive, OnMiddleButtonUp)
CREATE_EVENT_HANDLER(OnRightButtonDownRecursive, OnRightButtonDown)
CREATE_EVENT_HANDLER(OnRightButtonUpRecursive, OnRightButtonUp)
CREATE_EVENT_HANDLER(OnMouseMoveRecursive, OnMouseMove)
CREATE_EVENT_HANDLER(OnMouseWheelRecursive, OnMouseWheel)
CREATE_EVENT_HANDLER(OnKeyDownRecursive, OnKeyDown)
CREATE_EVENT_HANDLER(OnKeyUpRecursive, OnKeyUp)
CREATE_EVENT_HANDLER(OnDrawRecursive, OnDraw)

void ModeBase::OnUpdateMenuRecursive(bool multi_view_handled)
{
	if (parent)
		parent->OnUpdateMenuRecursive();
	OnUpdateMenu();
}

void ModeBase::OnDrawWinRecursive(MultiView::Window *win)
{
	if (parent)
		parent->OnDrawWinRecursive(win);
	OnDrawWin(win);
}

void ModeBase::OptimizeViewRecursice()
{
	if (OptimizeView())
		return;
	if (parent)
		parent->OptimizeViewRecursice();
}



ModeBase *ModeBase::GetRoot()
{
	if (parent)
		return parent->GetRoot();
	return this;
}

bool ModeBase::IsAncestorOf(ModeBase *m)
{
	if (m == this)
		return true;
	if (m->parent)
		return IsAncestorOf(m->parent);
	return false;
}


ModeBase *ModeBase::GetNextChildTo(ModeBase *target)
{
	while(target){
		if (this == target->parent)
			return target;
		target = target->parent;
	}
	return NULL;
}


bool ModeBase::EqualRoots(ModeBase *m)
{
	if (!m)
		return false;
	return GetRoot() == m->GetRoot();
}

void ModeBase::New()
{
	if (parent)
		parent->New();
}

bool ModeBase::Save()
{
	if (parent)
		return parent->Save();
	return true;
}



bool ModeBase::SaveAs()
{
	if (parent)
		return parent->SaveAs();
	return true;
}



bool ModeBase::Open()
{
	if (parent)
		return parent->Open();
	return true;
}





