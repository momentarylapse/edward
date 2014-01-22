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
