/*
 * AdminFileList.cpp
 *
 *  Created on: 02.03.2013
 *      Author: michi
 */

#include "AdminFileList.h"
#include "AdminFile.h"
#include "GameIniData.h"
#include "../../Edward.h"

AdminFile *AdminFileList::get(int kind, const string &name)
{
	string _name = name.sys_filename();
	foreach(AdminFile *a, *this)
		if ((a->Kind == kind) && (a->Name == _name))
			return a;
	return NULL;
}



AdminFile *AdminFileList::add_unchecked(int kind, const string &filename, AdminFile *source)
{
	if (filename.num <= 0)
		return NULL;
	msg_db_r("AddAdminFileUnchecked",5);
	msg_db_m(filename.c_str(),5);

	string _filename = filename.sys_filename();

	// is there already an entry in the database?
	AdminFile *a = get(kind, _filename);

	// no list entry yet -> create one
	if (!a){
		a = new AdminFile(_filename, kind);
		add(a);
	}

	// link to meta
	if (source)
		source->add_child(a);

	msg_db_l(5);
	return a;
}

// same as AddAdminFileUnchecked but used without file extensions
AdminFile *AdminFileList::add_unchecked_ae(int kind, const string &filename, AdminFile *source)
{
	if (filename.num<=0)
		return NULL;
	string filename2 = filename;
	if (kind==FDWorld)		filename2 += ".world";
	if (kind==FDTerrain)	filename2 += ".map";
	if (kind==FDModel)		filename2 += ".model";
	if (kind==FDMaterial)	filename2 += ".material";
	if (kind==FDFont)		filename2 += ".xfont";
	if (kind==FDCameraFlight)filename2 += ".camera";
	return add_unchecked(kind, filename2, source);
}

void AdminFileList::remove_obsolete()
{
	foreachib(AdminFile *a, *this, i){
		// missing and unwanted -> remove
		if ((a->Missing) && (a->Parent.num == 0))
			if (a->Kind >= 0) // don't remove engine files...
				erase(i);
	}
}

void AdminFileList::add_recursive(AdminFile *to_add)
{
	// already in list?
	for (int i=0;i<num;i++)
		if ((*this)[i] == to_add)
			return;

	// add
	add(to_add);

	// recursion...
	foreach(AdminFile *a, to_add->Child)
		add_recursive(a);
}

void AdminFileList::sort()
{
	msg_db_r("AdminFileList.sort",1);

	// sorting (by type)
	for (int i=0;i<num-1;i++)
		for (int j=i;j<num;j++)
			if ((*this)[i]->Kind > (*this)[j]->Kind)
				swap(i, j);
	// sorting (by name)
	for (int i=0;i<num-1;i++)
		if ((*this)[i]->Kind>=0)
			for (int j=i;j<num;j++)
				if ((*this)[i]->Kind == (*this)[j]->Kind)
					if ((*this)[i]->Name.compare((*this)[j]->Name) > 0)
						swap(i, j);
	msg_db_l(1);
}

void AdminFileList::clear()
{
	Array<AdminFile*>::clear();
}

void AdminFileList::clear_deep()
{
	foreach(AdminFile *a, *this)
		delete(a);
	Array<AdminFile*>::clear();
}


