/*
 * Mode.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Mode.h"
#include "../MultiView/MultiView.h"

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

void ModeBase::onUpdateMenuRecursive()
{
	if (parent)
		parent->onUpdateMenuRecursive();
	onUpdateMenu();
}

void ModeBase::onCommandRecursive(const string &id)
{
	if (parent)
		parent->onCommandRecursive(id);
	onCommand(id);
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





