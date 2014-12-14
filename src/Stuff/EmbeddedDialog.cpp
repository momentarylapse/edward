/*
 * EmbeddedDialog.cpp
 *
 *  Created on: 16.09.2013
 *      Author: michi
 */

#include "EmbeddedDialog.h"
#include "../lib/math/math.h"


EmbeddedDialog::EmbeddedDialog(HuiWindow *_win, const string &target_id, int x, int y, const string &options)
{
	Init(_win, target_id, x, y, options);
}

EmbeddedDialog::EmbeddedDialog(HuiWindow *_win, const string &dialog_id, const string &target_id, int x, int y, const string &options)
{
	Init(_win, target_id, x, y, options);
	win->embedDialog(dialog_id, 0, 0);
}

void EmbeddedDialog::Init(HuiWindow *_win, const string &target_id, int x, int y, const string &options)
{
	win = _win;
	uid = "embedded-" + i2s(randi(1000000));
	win->setBorderWidth(5);
	win->setTarget(target_id, 0);
	if (options.num > 0)
		win->addControlTable("!" + options, x, y, 1, 1, uid);
	else
		win->addControlTable("", x, y, 1, 1, uid);
	win->setTarget(uid, 0);
}

EmbeddedDialog::~EmbeddedDialog()
{
	win->removeEventHandlers(this);
	win->deleteControl(uid);
}

void EmbeddedDialog::enable(const string& id, bool enabled)
{
	win->enable(id, enabled);
}

void EmbeddedDialog::check(const string& id, bool checked)
{
	win->check(id, checked);
}

bool EmbeddedDialog::isChecked(const string& id)
{
	return win->isChecked(id);
}

void EmbeddedDialog::reset(const string& id)
{
	win->reset(id);
}

void EmbeddedDialog::setString(const string& id, const string& str)
{
	win->setString(id, str);
}

void EmbeddedDialog::addString(const string& id, const string& str)
{
	win->addString(id, str);
}

void EmbeddedDialog::setFloat(const string& id, float f)
{
	win->setFloat(id, f);
}

void EmbeddedDialog::setInt(const string& id, int i)
{
	win->setInt(id, i);
}

string EmbeddedDialog::getString(const string& id)
{
	return win->getString(id);
}

string EmbeddedDialog::getCell(const string& id, int row, int col)
{
	return win->getCell(id, row, col);
}

float EmbeddedDialog::getFloat(const string& id)
{
	return win->getFloat(id);
}

int EmbeddedDialog::getInt(const string& id)
{
	return win->getInt(id);
}
