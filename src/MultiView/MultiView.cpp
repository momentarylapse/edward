/*
 * MultiView.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../Edward.h"
#include "MultiView.h"
#include "Window.h"
#include "ActionController.h"
#include "SingleData.h"

namespace MultiView{

const string MultiView::MESSAGE_UPDATE = "Update";
const string MultiView::MESSAGE_SELECTION_CHANGE = "SelectionChange";
const string MultiView::MESSAGE_SETTINGS_CHANGE = "SettingsChange";
const string MultiView::MESSAGE_ACTION_START = "ActionStart";
const string MultiView::MESSAGE_ACTION_UPDATE = "ActionUpdate";
const string MultiView::MESSAGE_ACTION_ABORT = "ActionAbort";
const string MultiView::MESSAGE_ACTION_EXECUTE = "ActionExecute";


void MultiView::Selection::reset()
{
	index = set = type = -1;
}

MultiView::MultiView(bool mode3d) :
	Observable("MultiView")
{
}

MultiView::~MultiView()
{
}


};
