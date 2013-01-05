/*
 * Mode.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Mode.h"

Mode::Mode(const string &_name, Mode *_parent, Data *_data, MultiView *_multi_view, const string &_menu)
{
	parent = _parent;
	name = _name;
	data_generic = _data;
	multi_view = _multi_view;
	menu = NULL;
	if (_menu.num > 0)
		menu = HuiCreateResourceMenu(_menu);
	/*if ((!menu) && (parent))
		menu = parent->menu;*/
}

Mode::~Mode()
{
}

void Mode::OnCommandRecursive(const string & id, bool multi_view_handled)
{
	if ((multi_view) && (!multi_view_handled)){
		multi_view->OnCommand(id);
		multi_view_handled = true;
	}
	if (parent)
		parent->OnCommandRecursive(id, multi_view_handled);
	OnCommand(id);
}

#define CREATE_EVENT_HANDLER(_name_rec_, _name_)	\
void Mode::_name_rec_(bool multi_view_handled) \
{ \
	if ((multi_view) && (!multi_view_handled)){ \
		multi_view->_name_(); \
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
CREATE_EVENT_HANDLER(OnKeyDownRecursive, OnKeyDown)
CREATE_EVENT_HANDLER(OnKeyUpRecursive, OnKeyUp)
CREATE_EVENT_HANDLER(OnDrawRecursive, OnDraw)

void Mode::OnUpdateMenuRecursive(bool multi_view_handled)
{
	if (parent)
		parent->OnUpdateMenuRecursive();
	OnUpdateMenu();
}

void Mode::OnDrawWinRecursive(int win)
{
	if (parent)
		parent->OnDrawWinRecursive(win);
	OnDrawWin(win);
}



Mode *Mode::GetRoot()
{
	if (parent)
		return parent->GetRoot();
	return this;
}

bool Mode::IsAncestorOf(Mode *m)
{
	if (m == this)
		return true;
	if (m->parent)
		return IsAncestorOf(m->parent);
	return false;
}


Mode *Mode::GetNextChildTo(Mode *target)
{
	while(target){
		if (this == target->parent)
			return target;
		target = target->parent;
	}
	return NULL;
}


bool Mode::EqualRoots(Mode *m)
{
	if (!m)
		return false;
	return GetRoot() == m->GetRoot();
}

void Mode::New()
{
	if (parent)
		parent->New();
}

bool Mode::Save()
{
	if (parent)
		return parent->Save();
	return true;
}



bool Mode::SaveAs()
{
	if (parent)
		return parent->SaveAs();
	return true;
}



bool Mode::Open()
{
	if (parent)
		return parent->Open();
	return true;
}





