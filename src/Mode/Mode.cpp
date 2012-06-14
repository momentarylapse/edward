/*
 * Mode.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Mode.h"

Mode::Mode()
{
	menu = NULL;
	parent = NULL;
	multi_view = NULL;
}

Mode::~Mode()
{
}

void Mode::OnCommandRecursive(const string & id)
{
	if (multi_view)
		multi_view->OnCommand(id);
	if (parent)
		parent->OnCommandRecursive(id);
	OnCommand(id);
}

void Mode::OnLeftButtonUpRecursive()
{
	if (multi_view)
		multi_view->OnLeftButtonUp();
	if (parent)
		parent->OnLeftButtonUpRecursive();
	OnLeftButtonUp();
}

void Mode::OnMiddleButtonUpRecursive()
{
	if (multi_view)
		multi_view->OnMiddleButtonUp();
	if (parent)
		parent->OnMiddleButtonUpRecursive();
	OnMiddleButtonUp();
}

void Mode::OnKeyDownRecursive()
{
	if (multi_view)
		multi_view->OnKeyDown();
	if (parent)
		parent->OnKeyDownRecursive();
	OnKeyDown();
}

void Mode::OnRightButtonUpRecursive()
{
	if (multi_view)
		multi_view->OnRightButtonUp();
	if (parent)
		parent->OnRightButtonUpRecursive();
	OnRightButtonUp();
}

void Mode::OnKeyUpRecursive()
{
	if (multi_view)
		multi_view->OnKeyUp();
	if (parent)
		parent->OnKeyUpRecursive();
	OnKeyUp();
}

void Mode::OnMouseMoveRecursive()
{
	if (multi_view)
		multi_view->OnMouseMove();
	if (parent)
		parent->OnMouseMoveRecursive();
	OnMouseMove();
}

void Mode::OnLeftButtonDownRecursive()
{
	if (multi_view)
		multi_view->OnLeftButtonDown();
	if (parent)
		parent->OnLeftButtonDownRecursive();
	OnLeftButtonDown();
}

void Mode::OnMiddleButtonDownRecursive()
{
	if (multi_view)
		multi_view->OnMiddleButtonDown();
	if (parent)
		parent->OnMiddleButtonDownRecursive();
	OnMiddleButtonDown();
}

void Mode::OnRightButtonDownRecursive()
{
	if (multi_view)
		multi_view->OnRightButtonDown();
	if (parent)
		parent->OnRightButtonDownRecursive();
	OnRightButtonDown();
}

void Mode::OnUpdateMenuRecursive()
{
	if (parent)
		parent->OnUpdateMenuRecursive();
	OnUpdateMenu();
}

void Mode::OnDrawRecursive()
{
	if (parent)
		parent->OnDrawRecursive();
	OnDraw();
}

void Mode::OnDrawWinRecursive(int win, irect dest)
{
	if (parent)
		parent->OnDrawWinRecursive(win, dest);
	OnDrawWin(win, dest);
}



Mode *Mode::GetRootMode()
{
	if (parent)
		return parent->GetRootMode();
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





