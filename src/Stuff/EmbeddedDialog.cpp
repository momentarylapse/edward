/*
 * EmbeddedDialog.cpp
 *
 *  Created on: 16.09.2013
 *      Author: michi
 */

#include "EmbeddedDialog.h"
#include "../lib/math/math.h"


EmbeddedDialog::EmbeddedDialog(HuiWindow *_win, const string &target_id, int x, int y, int width, int height)
{
	Init(_win, target_id, x, y, width, height);
}

EmbeddedDialog::EmbeddedDialog(HuiWindow *_win, const string &dialog_id, const string &target_id, int x, int y, int width, int height)
{
	Init(_win, target_id, x, y, width, height);
	win->EmbedDialog(dialog_id, 0, 0);
}

void EmbeddedDialog::Init(HuiWindow *_win, const string &target_id, int x, int y, int width, int height)
{
	win = _win;
	uid = "embedded-" + i2s(randi(1000000));
	win->SetBorderWidth(5);
	win->SetTarget(target_id, 0);
	if (width > 0)
		win->AddControlTable("!width=" + i2s(width), x, y, 1, 1, uid);
	else
		win->AddControlTable("!width=250", x, y, 1, 1, uid);
	win->SetTarget(uid, 0);
}

EmbeddedDialog::~EmbeddedDialog()
{
	win->RemoveEventHandlers(this);
	win->DeleteControl(uid);
}

void EmbeddedDialog::Enable(const string& id, bool enabled)
{
	win->Enable(id, enabled);
}

void EmbeddedDialog::Check(const string& id, bool checked)
{
	win->Check(id, checked);
}

bool EmbeddedDialog::IsChecked(const string& id)
{
	return win->IsChecked(id);
}

void EmbeddedDialog::Reset(const string& id)
{
	win->Reset(id);
}

void EmbeddedDialog::SetString(const string& id, const string& str)
{
	win->SetString(id, str);
}

void EmbeddedDialog::AddString(const string& id, const string& str)
{
	win->AddString(id, str);
}

void EmbeddedDialog::SetFloat(const string& id, float f)
{
	win->SetFloat(id, f);
}

void EmbeddedDialog::SetInt(const string& id, int i)
{
	win->SetInt(id, i);
}

string EmbeddedDialog::GetString(const string& id)
{
	return win->GetString(id);
}

string EmbeddedDialog::GetCell(const string& id, int row, int col)
{
	return win->GetCell(id, row, col);
}

float EmbeddedDialog::GetFloat(const string& id)
{
	return win->GetFloat(id);
}

int EmbeddedDialog::GetInt(const string& id)
{
	return win->GetInt(id);
}
