/*
 * AdminFile.h
 *
 *  Created on: 02.03.2013
 *      Author: michi
 */

#ifndef ADMINFILE_H_
#define ADMINFILE_H_

#include "../../data/Data.h"

class AdminFileList;

class AdminFile{
public:
	Path Name;
	int Kind;
	bool Missing;
	bool Checked;
	Array<AdminFile*> Parent;
	Array<AdminFile*> Child;
	int Time;

	AdminFile();
	AdminFile(const Path &filename, int kind);
	void add_child(AdminFile *a);
	void remove_child(AdminFile *a);
	void remove_all_children();
	void check(Session *s, AdminFileList &list);
};


#endif /* ADMINFILE_H_ */
