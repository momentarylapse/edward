/*
 * AdminFile.h
 *
 *  Created on: 02.03.2013
 *      Author: michi
 */

#ifndef ADMINFILE_H_
#define ADMINFILE_H_

#include "../Data.h"

class AdminFileList;

struct AdminFile{
	string Name;
	int Kind;
	bool Missing;
	bool Checked;
	Array<AdminFile*> Parent;
	Array<AdminFile*> Child;
	int Time;

	AdminFile();
	AdminFile(const string &filename, int kind);
	void add_child(AdminFile *a);
	void remove_child(AdminFile *a);
	void remove_all_children();
	void check(AdminFileList &list);
};


#endif /* ADMINFILE_H_ */
