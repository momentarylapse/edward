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

color MultiView::ColorBackGround;
color MultiView::ColorBackGroundSelected;
color MultiView::ColorGrid;
color MultiView::ColorText;
color MultiView::ColorWindowType;
color MultiView::ColorPoint;
color MultiView::ColorPointSelected;
color MultiView::ColorPointSpecial;
color MultiView::ColorWindowSeparator;
color MultiView::ColorSelectionRect;
color MultiView::ColorSelectionRectBoundary;


void MultiView::Selection::reset()
{
	index = set = type = -1;
}

MultiView::MultiView(bool mode3d) :
	Observable("MultiView")
{
	/*ColorBackGround3D = color(1,0,0,0.15f);
	ColorBackGround2D = color(1,0,0,0.10f);
	ColorGrid = color(1,0.7f,0.7f,0.7f);
	ColorText = White;
	ColorWindowType = color(1, 0.5f, 0.5f, 0.5f);
	ColorPoint = color(1, 0.2f, 0.2f, 0.9f);
	ColorPointSelected = color(1, 0.9f, 0.2f, 0.2f);
	ColorPointSpecial = color(1, 0.2f, 0.8f, 0.2f);
	ColorWindowSeparator = color(1, 0.1f, 0.1f, 0.6f); // color(1,0.1f,0.1f,0.5f)
	ColorSelectionRect = color(0.2f,0,0,1);
	ColorSelectionRectBoundary = color(0.7f,0,0,1);*/
	ColorBackGround = color(1,0.9f,0.9f,0.9f);
	ColorBackGroundSelected = color(1,0.96f,0.96f,0.96f);
	ColorGrid = color(1,0.5f,0.5f,0.5f);
	ColorText = Black;
	ColorWindowType = color(1, 0.5f, 0.5f, 0.5f);
	ColorPoint = color(1, 0.2f, 0.2f, 0.9f);
	ColorPointSelected = color(1, 0.9f, 0.2f, 0.2f);
	ColorPointSpecial = color(1, 0.2f, 0.8f, 0.2f);
	ColorWindowSeparator = color(1, 0.1f, 0.1f, 0.6f); // color(1,0.1f,0.1f,0.5f)
	ColorSelectionRect = color(0.2f,0,0,1);
	ColorSelectionRectBoundary = color(0.7f,0,0,1);
}

MultiView::~MultiView()
{
}


};
