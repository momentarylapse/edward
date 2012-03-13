/*
 * ActionModel__SurfaceDeleteTriangle.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModel__SurfaceDeleteTriangle.h"
#include "../../Data/Model/DataModel.h"

ActionModel__SurfaceDeleteTriangle::ActionModel__SurfaceDeleteTriangle(int _surface, int _index)
{
	surface = _surface;
	index = _index;
}

ActionModel__SurfaceDeleteTriangle::~ActionModel__SurfaceDeleteTriangle()
{
	// TODO Auto-generated destructor stub
}

void ActionModel__SurfaceDeleteTriangle::undo(Data *d)
{
}



void *ActionModel__SurfaceDeleteTriangle::execute(Data *d)
{
}



void ActionModel__SurfaceDeleteTriangle::redo(Data *d)
{
}


