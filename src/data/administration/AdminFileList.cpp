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
#include "../../storage/Storage.h"

AdminFile *AdminFileList::get(int kind, const Path &name) {
	for (AdminFile *a: *this)
		if ((a->Kind == kind) and (a->Name == name))
			return a;
	return NULL;
}



AdminFile *AdminFileList::add_unchecked(int kind, const Path &filename, AdminFile *source)
{
	if (filename.is_empty())
		return nullptr;

	// is there already an entry in the database?
	AdminFile *a = get(kind, filename);

	// no list entry yet -> create one
	if (!a){
		a = new AdminFile(filename, kind);
		add(a);
	}

	// link to meta
	if (source)
		source->add_child(a);
	return a;
}

Path add_ext(const Path &p, const string &ext) {
	return p.str() + ext;
}

// same as AddAdminFileUnchecked but used without file extensions
AdminFile *AdminFileList::add_unchecked_ae(int kind, const Path &filename, AdminFile *source) {
	if (filename.is_empty())
		return NULL;
	string ext;
	if (kind==FD_WORLD)		ext = ".world";
	if (kind==FD_TERRAIN)	ext = ".map";
	if (kind==FD_MODEL)		ext = ".model";
	if (kind==FD_MATERIAL)	ext = ".material";
	if (kind==FD_FONT)		ext = ".xfont";
	if (kind==FD_CAMERAFLIGHT)ext = ".camera";
	return add_unchecked(kind, add_ext(filename, ext), source);
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
	for (AdminFile *a: to_add->Child)
		add_recursive(a);
}

void AdminFileList::sort()
{
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
}

void AdminFileList::clear()
{
	Array<AdminFile*>::clear();
}

void AdminFileList::clear_deep()
{
	for (AdminFile *a: *this)
		delete(a);
	Array<AdminFile*>::clear();
}


