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
	OnCommand(id);
}

void Mode::OnPreLeftButtonUp()
{
	if (multi_view)
		multi_view->OnLeftButtonUp();
	OnLeftButtonUp();
}

void Mode::OnPreMiddleButtonUp()
{
	if (multi_view)
		multi_view->OnMiddleButtonUp();
	OnMiddleButtonUp();
}

void Mode::OnPreKeyDown()
{
	if (multi_view)
		multi_view->OnKeyDown();
	OnKeyDown();
}

void Mode::OnPreRightButtonUp()
{
	if (multi_view)
		multi_view->OnRightButtonUp();
	OnRightButtonUp();
}

void Mode::OnPreKeyUp()
{
	if (multi_view)
		multi_view->OnKeyUp();
	OnKeyUp();
}

void Mode::OnPreMouseMove()
{
	if (multi_view)
		multi_view->OnMouseMove();
	OnMouseMove();
}

void Mode::OnPreLeftButtonDown()
{
	if (multi_view)
		multi_view->OnLeftButtonDown();
	OnLeftButtonDown();
}

void Mode::OnPreMiddleButtonDown()
{
	if (multi_view)
		multi_view->OnMiddleButtonDown();
	OnMiddleButtonDown();
}

void Mode::OnPreRightButtonDown()
{
	if (multi_view)
		multi_view->OnRightButtonDown();
	OnRightButtonDown();
}



