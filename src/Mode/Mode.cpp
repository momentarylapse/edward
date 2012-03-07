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
}

Mode::~Mode()
{
}

void Mode::OnPreCommand(const string & id)
{
	if (multi_view)
		multi_view->OnCommand(id);
	if (parent)
		parent->OnPreCommand(id);
	OnCommand(id);
}

void Mode::OnPreLeftButtonUp()
{
	if (multi_view)
		multi_view->OnLeftButtonUp();
	if (parent)
		parent->OnPreLeftButtonUp();
	OnLeftButtonUp();
}

void Mode::OnPreMiddleButtonUp()
{
	if (multi_view)
		multi_view->OnMiddleButtonUp();
	if (parent)
		parent->OnPreMiddleButtonUp();
	OnMiddleButtonUp();
}

void Mode::OnPreKeyDown()
{
	if (multi_view)
		multi_view->OnKeyDown();
	if (parent)
		parent->OnPreKeyDown();
	OnKeyDown();
}

void Mode::OnPreRightButtonUp()
{
	if (multi_view)
		multi_view->OnRightButtonUp();
	if (parent)
		parent->OnPreRightButtonUp();
	OnRightButtonUp();
}

void Mode::OnPreKeyUp()
{
	if (multi_view)
		multi_view->OnKeyUp();
	if (parent)
		parent->OnPreKeyUp();
	OnKeyUp();
}

void Mode::OnPreMouseMove()
{
	if (multi_view)
		multi_view->OnMouseMove();
	if (parent)
		parent->OnPreMouseMove();
	OnMouseMove();
}

void Mode::OnPreLeftButtonDown()
{
	if (multi_view)
		multi_view->OnLeftButtonDown();
	if (parent)
		parent->OnPreLeftButtonDown();
	OnLeftButtonDown();
}

void Mode::OnPreMiddleButtonDown()
{
	if (multi_view)
		multi_view->OnMiddleButtonDown();
	if (parent)
		parent->OnPreMiddleButtonDown();
	OnMiddleButtonDown();
}

void Mode::OnPreRightButtonDown()
{
	if (multi_view)
		multi_view->OnRightButtonDown();
	if (parent)
		parent->OnPreRightButtonDown();
	OnRightButtonDown();
}

void Mode::OnPreDataChange()
{
	if (parent)
		parent->OnPreDataChange();
	OnDataChange();
}



