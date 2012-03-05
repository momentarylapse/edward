#include "file.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

#ifdef FILE_OS_WINDOWS
	#include <io.h>
	#include <direct.h>
#endif
#ifdef FILE_OS_LINUX
	#include <unistd.h>
	#include <dirent.h>
	//#include <sys/timeb.h>
 
	 
	#define _open	open
	#define _read	read
	#define _write	write
	#define _lseek	lseek
	#define _close	close
	#define _rmdir	rmdir
	#define _unlink	unlink
	inline unsigned char to_low(unsigned char c)
	{
		if ((c>='A')&&(c<='Z'))
			return c-'A'+'a';
		return c;
	}
	int _stricmp(const char*a,const char*b)
	{
		unsigned char a_low=to_low(*a);
		unsigned char b_low=to_low(*b);
		while((*a!=0)&&(*b!=0)){
			if (a_low!=b_low)	break;
			a++,++b;
			a_low=to_low(*a);
			b_low=to_low(*b);
		}
		return a_low-b_low;
	}
#endif



static CFile *test_file=NULL;

// just test the existence of a file
bool file_test_existence(const string &filename)
{
	if (!test_file)
		test_file=new CFile();
	test_file->SilentFileAccess=test_file->DontReportErrors=true;
	if (!test_file->Open(filename)){
		//delete(test_file);
		return false;
	}
	test_file->Close();
	return true;
}


bool dir_create(const string &dir)
{
#ifdef FILE_OS_WINDOWS
	return (_mkdir(SysFileName(dir).c_str())==0);
#endif
#ifdef FILE_OS_LINUX
	return (mkdir(SysFileName(dir).c_str(),S_IRWXU | S_IRWXG | S_IRWXO)==0);
#endif
	return false;
}

bool dir_delete(const string &dir)
{
	return (_rmdir(SysFileName(dir).c_str())==0);
}

string get_current_dir()
{
	string str;
	char tmp[256];
#ifdef FILE_OS_WINDOWS
	char *r=_getcwd(tmp, sizeof(tmp));
	str = tmp;
	str += "\\";
#endif
#ifdef FILE_OS_LINUX
	char *r=getcwd(tmp, sizeof(tmp));
	str = tmp;
	str += "/";
#endif
	return str;
}

bool file_rename(const string &source,const string &target)
{
	char dir[512];
	for (unsigned int i=0;i<target.num;i++){
		dir[i]=target[i];
		dir[i+1]=0;
		if (i>3)
			if ((target[i]=='/')||(target[i]=='\\'))
				dir_create(string(dir));
	}
	return (rename(SysFileName(source).c_str(), SysFileName(target).c_str())==0);
}

bool file_copy(const string &source,const string &target)
{
	char dir[512];
	for (unsigned int i=0;i<target.num;i++){
		dir[i]=target[i];
		dir[i+1]=0;
		if (i>3)
			if ((target[i]=='/')||(target[i]=='\\'))
				dir_create(string(dir));
	}
	int hs=_open(SysFileName(source).c_str(),O_RDONLY);
	if (hs<0)
		return false;
#ifdef FILE_OS_WINDOWS
	int ht=_creat(SysFileName(target).c_str(),_S_IREAD | _S_IWRITE);
	_setmode(hs,_O_BINARY);
	_setmode(ht,_O_BINARY);
#endif
#ifdef FILE_OS_LINUX
	int ht=creat(SysFileName(target).c_str(),S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#endif
	if (ht<0){
		_close(hs);
		return false;
	}
	char buf[10240];
	int r=10;
	while(r>0){
		r=_read(hs,buf,sizeof(buf));
		int rr=_write(ht,buf,r);
	}
	_close(hs);
	_close(ht);
	return true;
}

bool file_delete(const string &filename)
{
	return (_unlink(SysFileName(filename).c_str())==0);
}

string shell_execute(const string &cmd)
{
	system(cmd.c_str());
	return "";
}


Array<string> dir_search_name;
Array<bool> dir_search_is_dir;

inline void dir_search_add(string &name, bool is_dir)
{
	dir_search_name.add(name);
	dir_search_is_dir.add(is_dir);
}

// seach an directory for files matching a filter
int dir_search(const string &dir, const string &filter, bool show_directories)
{
	msg_db_r("dir_search", 1);
	for (int i=0;i<dir_search_name.num;i++)
		dir_search_name[i].clear();
	dir_search_name.clear();
	dir_search_is_dir.clear();


	string filter2 = filter.substr(1, filter.num - 1);
	string dir2 = dir;
	dir_ensure_ending(dir2, true);

#ifdef FILE_OS_WINDOWS
	static _finddata_t t;
	int handle=_findfirst(SysFileName(dir2 + "*").c_str(), &t);
	int e=handle;
	while(e>=0){
		//if ((strcmp(t.name,".")!=0)&&(strcmp(t.name,"..")!=0)&&(strstr(t.name,"~")==NULL)){
		if ((t.name[0]!='.')&&(!strstr(t.name,"~"))){
			string fn = string(t.name);
			if ((fn.find(filter2) >= 0)|| ((show_directories)&&(t.attrib==_A_SUBDIR)) ){
				dir_search_add(fn, (t.attrib == _A_SUBDIR));
			}
		}
		e=_findnext(handle,&t);
	}
#endif
#ifdef FILE_OS_LINUX
	DIR *_dir;
	_dir=opendir(SysFileName(dir2).c_str());
	if (!_dir){
		msg_db_l(1);
		return 0;
	}
	struct dirent *dn;
	dn=readdir(_dir);
	struct stat s;
	while(dn){
		//if ((strcmp(dn->d_name,".")!=0)&&(strcmp(dn->d_name,"..")!=0)&&(!strstr(dn->d_name,"~"))){
			if ((dn->d_name[0]!='.')&&(!strstr(dn->d_name,"~"))){
				string ffn = SysFileName(dir2) + dn->d_name;
				stat(ffn.c_str(), &s);
				bool is_reg=(s.st_mode & S_IFREG)>0;
				bool is_dir=(s.st_mode & S_IFDIR)>0;
				int sss=strlen(dn->d_name) - filter2.num;
				if (sss<0)	sss=0;
				if ( ((is_reg)&&(strcmp(&dn->d_name[sss],filter2.c_str())==0)) || ((show_directories)&&(is_dir)) ){
					string fn = string(dn->d_name);
					dir_search_add(fn, is_dir);
				}
			}
			dn=readdir(_dir);
	}
	closedir(_dir);
#endif

	
	// sorting...
	for (int i=0;i<dir_search_name.num-1;i++)
		for (int j=i+1;j<dir_search_name.num;j++){
			bool ok = true;
			if (dir_search_is_dir[i] == dir_search_is_dir[j]){
				if (dir_search_name[i].icompare(dir_search_name[j]) > 0)
					ok = false;
			}else
				if ((!dir_search_is_dir[i]) && (dir_search_is_dir[j]))
					ok = false;
			if (!ok){
				dir_search_name.swap(i, j);
				dir_search_is_dir.swap(i, j);
			}
		}

	msg_db_l(1);
	return dir_search_name.num;
}



