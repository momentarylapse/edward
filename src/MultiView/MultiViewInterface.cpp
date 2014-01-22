/*
 * MultiViewInterface.cpp
 *
 *  Created on: 21.01.2014
 *      Author: michi
 */

#include "MultiViewInterface_.h"

namespace MultiView{

void MultiViewInterface::Selection::reset()
{
	index = set = type = -1;
}

MultiViewInterface::MultiViewInterface() :
	Observable("MultiView")
{
}

MultiViewInterface::~MultiViewInterface()
{
}

};
