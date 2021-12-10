/*----------------------------------------------------------------------------*\
| Object                                                                       |
| -> physical entities of a model in the game                                  |
| -> manages physics on its own                                                |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2009.12.03 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#pragma once

#include "Model.h"


class Object : public Model {
public:
	Object();
//	~Object();

	void make_visible(bool _visible_);
};


