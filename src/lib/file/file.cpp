/*----------------------------------------------------------------------------*\
| CFile                                                                        |
| -> acces to files (high & low level)                                         |
| -> text mode / binary mode                                                   |
|    -> textmode: numbers as decimal numbers, 1 line per value saved,          |
|                 carriage-return/linefeed 2 characters (windows),...          |
|    -> binary mode: numbers as 4 byte binary coded, carriage-return 1         |
|                    character,...                                             |
| -> opening a missing file can call a callback function (x: used for          |
|    automatically downloading the file)                                       |
| -> files can be stored in an archive file                                    |
|                                                                              |
| vital properties:                                                            |
|  - a single instance per file                                                |
|                                                                              |
| last update: 2010.06.01 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#include "file.h"



//#define StructuredShifts
//#define FILE_COMMENTS_DEBUG

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>

#ifdef FILE_OS_WINDOWS
	#include <stdio.h>
	#include <io.h>
	#include <direct.h>
	#include <stdarg.h>
	#include <windows.h>
	#include <winbase.h>
	#include <winnt.h>
#endif
#ifdef FILE_OS_LINUX
	#include <unistd.h>
	#include <dirent.h>
	#include <stdarg.h>
	#include <sys/timeb.h>
	#include <sys/stat.h>

	#define _open	open
	#define _read	read
	#define _write	write
	#define _lseek	lseek
	bool _eof(int handle)
	{
		int pos=lseek(handle,0,SEEK_CUR);
		struct stat stat;
		fstat(handle, &stat);
		return (pos>=stat.st_size);
	}
	#define _close	close
	#define _rmdir	rmdir
	#define _unlink	unlink
#endif

bool SilentFiles = false;


#ifdef FILE_OS_WINDOWS
Date time2date(_SYSTEMTIME t)
{
	Date d;
	d.time=-1;//(int)t.time;   :---(
	d.year=t.wYear;
	d.month=t.wMonth;
	d.day=t.wDay;
	d.hour=t.wHour;
	d.minute=t.wMinute;
	d.second=t.wSecond;
	d.milli_second=t.wMilliseconds;
	d.day_of_week=t.wDayOfWeek;
	d.day_of_year=-1;//t.wDayofyear;   :---(
	return d;
}
#endif
#ifdef FILE_OS_LINUX
Date time2date(time_t t)
{
	Date d;
	d.time=(int)t;
	tm *tm=localtime(&t);
	d.year=tm->tm_year+1900;
	d.month=tm->tm_mon;
	d.day=tm->tm_mday;
	d.hour=tm->tm_hour;
	d.minute=tm->tm_min;
	d.second=tm->tm_sec;
	d.milli_second=0;
	d.day_of_week=tm->tm_wday;
	d.day_of_year=tm->tm_yday;
	return d;
}
#endif

Date get_current_date()
{
#ifdef FILE_OS_WINDOWS
	_SYSTEMTIME t;
	GetLocalTime(&t);
	return time2date(t);
#endif
#ifdef FILE_OS_LINUX
	time_t t;
	t = time(NULL);
	Date d;
	d = time2date(t);
	timeb tb;
	ftime(&tb);
	d.milli_second = tb.millitm;
	return d;
#endif
}


t_file_try_again_func *FileTryAgainFunc;

static int a_num_dirs=-1,a_num_files=-1;
struct s_a_dir{
	string name;
	int first_file,num_files;
}*a_dir;
struct s_a_file{
	string name;
	int size,offset,dir_no;
	bool decompressed;
}*a_file;
static CFile *a_f=NULL;

static int a_num_created_dirs=0;
static string a_created_dir[256];

#if 0

// set and open the current archive
//   calls to CFile::Open will try to find a file inside this archive
void file_set_archive(const string &filename)
{
	if (a_f)
		file_clean_up_archive();
	a_f=new CFile();
	a_f->SilentFileAccess=true;
	if (!a_f->Open(filename)){
		delete(a_f);
		a_f=NULL;
		return;
	}
	a_f->SetBinaryMode(true);
	a_f->ReadInt(); // "michizip"
	a_f->ReadInt();
	int vers=a_f->ReadInt();
	int comp=a_f->ReadInt();
	if ((vers!=0)&&(comp!=1)){
		msg_error(format("unsupported archive: version %d, compression %d   (I need v0, c1)",vers,comp));
		a_f->Close();
		delete(a_f);
		a_f=NULL;
		return;
	}
	a_num_dirs=a_f->ReadInt();
	a_dir=new s_a_dir[a_num_dirs];
	a_num_files=a_f->ReadInt();
	a_file=new s_a_file[a_num_files];
	for (int i=0;i<a_num_dirs;i++){
		a_dir[i].name = SysFileName(a_f->ReadStr());
		a_dir[i].first_file=a_f->ReadInt();
		a_dir[i].num_files=a_f->ReadInt();
		for (int j=a_dir[i].first_file;j<a_dir[i].first_file+a_dir[i].num_files;j++)
			a_file[j].dir_no=i;
	}
	for (int i=0;i<a_num_files;i++){
		a_file[i].name = a_f->ReadStr();
		a_file[i].size=a_f->ReadInt();
		a_file[i].decompressed=false;
	}
	int offset=a_f->GetPos();
	for (int i=0;i<a_num_files;i++){
		a_file[i].offset=offset;
		offset+=a_file[i].size;
	}
	
//	a_f->Close();
//	delete(a_f);
}

struct sMzipTree{
	char data;
	int sub_tree[2],root;
};

static sMzipTree tree[768]; // 3*256 (using boring algorithm...)
static int num_trees;


static void read_tree(const char *in_buffer,int &pos,int i)
{
	if (in_buffer[pos++]==0){
		tree[i].sub_tree[0]=num_trees;
		read_tree(in_buffer,pos,num_trees++);
		tree[i].sub_tree[1]=num_trees;
		read_tree(in_buffer,pos,num_trees++);
	}else{
		tree[i].sub_tree[0]=tree[i].sub_tree[1]=-1;
		tree[i].data=in_buffer[pos++];
	}
}

void add_created_dir(const string &dir)
{
	for (int i=0;i<a_num_created_dirs;i++)
		if (a_created_dir[i] == SysFileName(dir))
			return;
	a_created_dir[a_num_created_dirs++] = SysFileName(dir);
}

// try to find a file inside the archive and create a decompressed copy
//   only relative paths supported!!!
bool file_get_from_archive(const char *filename)
{
	if (!a_f)
		return false;
	char dir[512],file[128];
	int i,j;
	// easify the name
	for (i=strlen(filename)-1;i>=0;i--)
		if ((filename[i]=='/')||(filename[i]=='\\')){
			dir,SysFileName(filename));
			dir[i+1]=0;
			strcpy(file,&filename[i+1]);
			break;
		}
	msg_write(string("trying to find file in archive: ",filename));
	// find the file inside the archive
	int nf=-1;
	for (i=0;i<a_num_dirs;i++)
		if (strcmp(dir,a_dir[i].name)==0){
			for (j=a_dir[i].first_file;j<a_dir[i].first_file+a_dir[i].num_files;j++)
				if (strcmp(file,a_file[j].name)==0){
					nf=j;
					break;
				}
			break;
		}
	if (nf<0)
		return false;
	// read the raw data
	char *in_buffer=new char[a_file[nf].size];
	a_f->SetPos(a_file[nf].offset,true);
	a_f->ReadBuffer(in_buffer,a_file[nf].size);
	// decompress
	int version=((int*)&in_buffer[0])[0];
	int out_size=((int*)&in_buffer[0])[1];
	char *out_buffer=new char[out_size];
	num_trees=1;
	int pos=8;
	read_tree(in_buffer,pos,0);
	//trees_out();
	int offset=0;
	for (i=0;i<out_size;i++){
		int t=0;
		while(true){
			if (tree[t].sub_tree[0]<0){
				out_buffer[i]=tree[t].data;
				break;
			}
			// read current bit
			t=tree[t].sub_tree[(int)( (in_buffer[pos]&(1<<offset))>0 )];
			offset++;
			if (offset>=8){
				offset=0;
				pos++;
			}
		}
	}
	// test directories
	for (unsigned int n=3;n<strlen(dir);n++){
		if ((dir[n]=='/')||(dir[n]=='\\')){
			char temp[256];
			strcpy(temp,dir);
			temp[n]=0;
			if (dir_create(temp))
				add_created_dir(temp);
		}
	}
	// write the file
	CFile *f=new CFile();
	f->SilentFileAccess=true;
	f->SetBinaryMode(true);
	f->Create(filename);
	f->SetBinaryMode(true);
	f->WriteBuffer(out_buffer,out_size);
	f->Close();
	delete(f);
	delete[](in_buffer);
	delete[](out_buffer);
	a_file[nf].decompressed=true;
	return true;
}

// remove all decompressed files (and directories...)
void file_clean_up_archive()
{
	if (a_f){
		msg_write("cleaning file archive");
		a_f->Close();
		delete(a_f);
		a_f=NULL;

		int i;
		for (i=0;i<a_num_files;i++)
			if (a_file[i].decompressed)
				file_delete(string(a_dir[a_file[i].dir_no].name,a_file[i].name));
		for (i=a_num_created_dirs-1;i>=0;i--)
			dir_delete(a_created_dir[i]);

		a_num_files=0;
		a_num_dirs=0;
		a_num_created_dirs=0;
		delete(a_file);
		delete(a_dir);
	}
}
#endif

void _file_set_error_(CFile *f, const string &str)
{
	msg_error("during file access: " + str);
	f->Error = f->ErrorReported = true;
}


CFile::CFile()
{
	Error = false;
	ErrorReported = false;
	SilentFileAccess = SilentFiles;
	DontReportErrors = false;
	handle = -1;
}

CFile::~CFile()
{
	if (handle>=0)
		Close();
}

CFile *OpenFile(const string &filename)
{
	CFile *f = new CFile();
	if (f->Open(filename))
		return f;
	delete(f);
	return NULL;
}

CFile *OpenFileSilent(const string &filename)
{
	CFile *f = new CFile();
	f->SilentFileAccess = true;
	if (f->Open(filename))
		return f;
	delete(f);
	return NULL;
}

CFile *CreateFile(const string &filename)
{
	CFile *f = new CFile();
	if (f->Create(filename))
		return f;
	delete(f);
	return NULL;
}

CFile *CreateFileSilent(const string &filename)
{
	CFile *f = new CFile();
	f->SilentFileAccess = true;
	if (f->Create(filename))
		return f;
	delete(f);
	return NULL;
}

CFile *AppendFile(const string &filename)
{
	CFile *f = new CFile();
	if (f->Append(filename))
		return f;
	delete(f);
	return NULL;
}

void FileClose(CFile *f)
{
	f->Close();
	delete(f);
}

string FileRead(const string &filename)
{
	CFile *f = OpenFile(filename);
	if (!f)
		return "";
	string r = f->ReadComplete();
	FileClose(f);
	return r;
}

void FileWrite(const string &filename, const string &str)
{
	CFile *f = CreateFile(filename);
	if (!f)
		return;
	f->WriteBuffer(str.data, str.num);
	FileClose(f);
}


// open a file
bool CFile::Open(const string &filename)
{
	if (!SilentFileAccess){
		msg_write("loading file: " + SysFileName(filename));
		msg_right();
	}
	Error=Eof=false;
	Reading = true;
	handle=_open(SysFileName(filename).c_str(),O_RDONLY);
	if (handle<=0){
		/*if (file_get_from_archive(filename)){
			handle=_open(SysFileName(filename).c_str(),O_RDONLY);
			SetBinaryMode(false);
			return true;
		}else*/ if (FileTryAgainFunc){
			if (FileTryAgainFunc(filename)){
				handle=_open(SysFileName(filename).c_str(),O_RDONLY);
				SetBinaryMode(false);
				return true;
			}
		}
		Error=ErrorReported=true;
		if (!DontReportErrors){
			if (SilentFileAccess)
				msg_error("while opening the file: " + filename);
			else
				msg_error("while opening the file");
		}
		if (!SilentFileAccess)
			msg_left();
	}else{
		SetBinaryMode(false);
	}
	return !Error;
}

// create a new file or reset an existing one
bool CFile::Create(const string &filename)
{
	if (!SilentFileAccess){
		msg_write("creating file: " + SysFileName(filename));
		msg_right();
	}
	Error=false;
	Reading = false;
	FloatDecimals=3;
#ifdef FILE_OS_WINDOWS
	handle=_creat(SysFileName(filename).c_str(),_S_IREAD | _S_IWRITE);
#endif
#ifdef FILE_OS_LINUX
	handle=creat(SysFileName(filename).c_str(),S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#endif
	if (handle<=0){
		Error=ErrorReported=true;
		if (!DontReportErrors){
			if (SilentFileAccess)
				msg_error("while creating the file: " + filename);
			else
				msg_error("while creating the file");
		}
		return false;
	}
	SetBinaryMode(false);
	return true;
}

// create a new file or append data to an existing one
bool CFile::Append(const string &filename)
{
	if (!SilentFileAccess){
		msg_write("appending file: " + SysFileName(filename));
		msg_right();
	}
	Error=false;
	Reading = false;
	FloatDecimals=3;
#ifdef FILE_OS_WINDOWS
	handle=_open(SysFileName(filename).c_str(),O_WRONLY | O_APPEND | O_CREAT,_S_IREAD | _S_IWRITE);
#endif
#ifdef FILE_OS_LINUX
	handle=open(SysFileName(filename).c_str(),O_WRONLY | O_APPEND | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#endif
	if (handle<=0){
		Error=ErrorReported=true;
		if (!DontReportErrors){
			if (SilentFileAccess)
				msg_error("while appending file: " + filename);
			else
				msg_error("while appending file");
		}
		return false;
	}
	SetBinaryMode(false);
	return true;
}

// close the file
bool _cdecl CFile::Close()
{
	//flush(handle);
	if (handle >= 0)
		_close(handle);
	handle = -1;
	if (SilentFileAccess){
		/*if (Error)
			msg_error("during file access");*/
	}else{
		if (!DontReportErrors){
			if ((Error)&&(!ErrorReported))
				msg_error("during file access");
			else
				msg_ok();
		}
		if (!SilentFileAccess)
			msg_left();
	}
	return Error;
}

// switch between text mode and binary mode
void CFile::SetBinaryMode(bool binary)
{
	if (Error)		return;
	if (handle<0)	return;
	Binary=binary;
#ifdef FILE_OS_WINDOWS
	if (binary)
		_setmode(handle,_O_BINARY);
	else
		_setmode(handle,_O_TEXT);
#endif
	// ignored by linux
	// (no fake return-characters!!!!!!)
}

// jump to an position in the file or to a position relative to the current
void CFile::SetPos(int pos,bool absolute)
{
	if (absolute)	_lseek(handle,pos,SEEK_SET);
	else			_lseek(handle,pos,SEEK_CUR);
}

// retrieve the size of the opened(!) file
int CFile::GetSize()
{
#ifdef FILE_OS_WINDOWS
	return (int)_filelength(handle);
#endif
#ifdef FILE_OS_LINUX
	struct stat stat;
	fstat(handle, &stat);
	return stat.st_size;
#endif
}

Date CFile::GetDate(int type)
{
	time_t t;
	struct stat stat;
	fstat(handle, &stat);
	if (type==FileDateAccess)		t=stat.st_atime;
	if (type==FileDateModification)	t=stat.st_mtime;
	if (type==FileDateCreation)		t=stat.st_ctime;
	//return time2date(t);

	Date d;
	tm *tm=localtime(&t);
	d.time=(int)t;
	d.year=tm->tm_year+1900;
	d.month=tm->tm_mon;
	d.day=tm->tm_mday;
	d.hour=tm->tm_hour;
	d.minute=tm->tm_min;
	d.second=tm->tm_sec;
	d.day_of_week=tm->tm_wday;
	d.day_of_year=tm->tm_yday;
	return d;
}

// where is the current reading position in the file?
int CFile::GetPos()
{
	return _lseek(handle,0,SEEK_CUR);
}

// read a single character followed by the file-format-version-number
int CFile::ReadFileFormatVersion()
{
	if (Error)	return -1;
	unsigned char a=0;
	if (_read(handle,&a,1)<=0){
		Eof=true;
		return -1;
	}
	if (a=='t')
		SetBinaryMode(false);
	else if (a=='b')
		SetBinaryMode(true);
	else{
		msg_error("File Format Version must begin ether with 't' or 'b'!!!");
		Error=true;
		return -1;
	}
	return ReadWord();
}

// write a single character followed by the file-format-version-number
void CFile::WriteFileFormatVersion(bool binary,int fvv)
{
	char a=binary?'b':'t';
	int r=_write(handle,&a,1);
	SetBinaryMode(binary);
	WriteWord(fvv);
}

#define chunk_size		2048
char chunk[chunk_size];

// read the complete file into the buffer
string CFile::ReadComplete()
{
	string buf;
	int t_len=chunk_size;
	while(t_len>0){
		t_len = _read(handle, chunk, chunk_size);
		if (t_len < 0){
			buf.clear();
			_file_set_error_(this,"r<0");
			return "";
		}
		int n0 = buf.num;
#ifdef FILE_OS_WINDOWS
		buf.resize(buf.num + t_len);
		memcpy(&((char*)buf.data)[n0], chunk, t_len);
#else
		if (Binary){
			buf.resize(buf.num + t_len);
			memcpy(&((char*)buf.data)[n0], chunk, t_len);
		}else{
			for (int i=0;i<t_len;i++)
				if (chunk[i]!='\r')
					buf.append_1_single(chunk[i]);
		}
#endif
	}
	return buf;
}

// read a part of the file into the buffer
int CFile::ReadBuffer(void *buffer,int size)
{
	return _read(handle,buffer,size);
/*	int t_len=chunk_size;
	size=0;
	while(t_len==chunk_size){
		t_len=read(handle,chunk,chunk_size);
#ifdef FILE_OS_WINDOWS
		memcpy(&buffer[size],chunk,t_len);
		size+=t_len;
#else
		if (!Binary){
			for (int i=0;i<t_len;i++)
				if (chunk[i]!='\r')
					buffer[size++]=chunk[i];
		}else{
			memcpy(&((char*)buffer)[size],chunk,t_len);
			size+=t_len;
		}
#endif
	}*/
}

// insert the buffer into the file
int CFile::WriteBuffer(const void *buffer,int size)
{
	return _write(handle,buffer,size);
}


// read a single character (1 byte)
char CFile::ReadChar()
{
	if (Error)	return 0;
	char c=0;
	if (_read(handle,&c,1)<1)
		Eof=true;
	return ((!Binary)&&(c==0x0d))?ReadChar():c;
}

// read a single character (1 byte)
unsigned char CFile::ReadByte()
{
	if (Error)	return 0;
	if (Binary){
		unsigned char a=0;
		if (_read(handle,&a,1)<1)
			Eof=true;
		return a;
	}
	return s2i(ReadStr());
}

unsigned char CFile::ReadByteC()
{
	ReadComment();
	return ReadByte();
}

// read the rest of the line (only text mode)
void CFile::ReadComment()
{
	if ((Error)||(Binary))	return;
#ifdef FILE_COMMENTS_DEBUG
	msg_write("comment: " + ReadStr());
#else
	unsigned char a=0;
	while (a!='\n'){
		if (_read(handle,&a,1)<=0){
			Error=true;
			Eof=true;
			break;
		}
	}
#endif

}

// read a word (2 bytes in binary mode)
unsigned short CFile::ReadWord()
{
	if (Error)	return 0;
	if (Binary){
		/*unsigned short w=0;
		if (_read(handle,&w,2)!=2)
			Eof=true;*/
		unsigned int w=0;
		if (_read(handle,&w,2)!=2)
			Eof=true;
		return w;
	}
	return s2i(ReadStr());
}

unsigned short CFile::ReadWordC()
{
	ReadComment();
	return ReadWord();
}

// read a word (2 bytes in binary mode)
unsigned short CFile::ReadReversedWord()
{
	if (Error)	return 0;
	if (Binary){
		int i=0;
		unsigned char a;
		if (_read(handle,&a,1)<=0){
			Error=true;
			Eof=true;
			return 0;
		}
		i=(int)(a);
		if (_read(handle,&a,1)<=0){
			Error=true;
			Eof=true;
			return 0;
		}
		i=i*256+(int)(a);
		return i;
	}
	return s2i(ReadStr());
}

// read an integer (4 bytes in binary mode)
int _cdecl CFile::ReadInt()
{
	if (Error)	return 0;
	if (Binary){
		int i;
		if (_read(handle,&i,4)!=4)
			Eof=true;
		return i;
	}
	return s2i(ReadStr());
}

// ignore this line, then read an integer
int _cdecl CFile::ReadIntC()
{
	ReadComment();
	return ReadInt();
}

// read a float (4 bytes in binary mode)
float CFile::ReadFloat()
{
	if (Error)	return 0;
	if (Binary){
		float f;
		if (_read(handle,&f,4)!=4)
			Eof=true;
		return f;
	}
	return s2f(ReadStr());
}

// ignore this line, then read an float
float CFile::ReadFloatC()
{
	ReadComment();
	return ReadFloat();
}

// read a boolean (1 byte in binary mode)
bool _cdecl CFile::ReadBool()
{
	if (Error)	return false;
	unsigned char a=0;
	bool res=false;
	if (_read(handle,&a,1)<=0){
		Error=true;
		Eof=true;
		return false;
	}
	if (a=='1')
		res=true;
	if (!Binary){
		unsigned char a=0;
		while (a!='\n'){
			if (_read(handle,&a,1)<=0){
				Error=true;
				Eof=true;
				break;
			}
		}
	}
	return res;
}

// ignore this line, then read an boolean
bool _cdecl CFile::ReadBoolC()
{
	ReadComment();
	return ReadBool();
}

// read a string
//   text mode:   complete rest of this line
//   binary mode: length word, then string
string _cdecl CFile::ReadStr()
{
	Error|=Eof;
	if (Error)	return "";

	if (Binary){
		// binary: read length as a word then read so many bytes
		int l = ReadWord();
		string str;
		str.resize(l + 16); // prevents "uninitialized" bytes in syscall parameter... (valgrind)
		if (_read(handle, (char*)str.data, l) < l){
			_file_set_error_(this,"ReadStr: _read()<1");
			Eof=(_eof(handle)>0);
			return "";
		}
		Eof=(_eof(handle)>0);
		str.resize(l);
		return str;
	}else{
		// read one byte at a time untill we reach a \n character
		char tttt[16];
		tttt[0]=-1;
		tttt[1]=0;
		string str;
		while(true){
			int rrr = _read(handle,tttt,1);

			// error?
			if (rrr <= 0){
				Eof=(_eof(handle)>0);
				if (!Eof){
					_file_set_error_(this,"ReadStr: read()<1");
					msg_write(":---(");
				}
				return "";
			}
			
			#ifdef FILE_OS_LINUX
				// windows read-function does this on its own... (O_O)
				if (tttt[0]=='\r')
					continue;
			#endif

				
			if ((tttt[0]=='\n') || (tttt[0]==0))
				return str;
			str.add(tttt[0]);
		}
	}
//	msg_write(string("(^.^)       ",str));
	return "";
}

// read a null-terminated string
string CFile::ReadStrNT()
{
	Error|=Eof;
	if (Error)	return "";
	string str;
	char a=-1;
	while(true){
		if (_read(handle,&a,1)<=0){
			Error=true;
			Eof=true;
			return "";
		}
		#ifdef FILE_OS_LINUX
			if (a=='\r')
				continue;
		#endif
		if (a == 0)
			return str;
		str.add(a);
	}
	return "";
}

// ignore this line, then read an string
string _cdecl CFile::ReadStrC()
{
	ReadComment();
	return ReadStr();
}

// read a string having reversed byte as length in binary mode
string CFile::ReadStrRW()
{
	Error|=Eof;
	if (Error)	return "";
	if (Binary){
		int l = ReadReversedWord();
		string str;
		str.resize(l);
		if (_read(handle,str.data,l) < l){
			Error=true;
			Eof=true;
			return "";
		}
		return str;
	}else{
		return ReadStr();
	}
}

void CFile::ReadVector(void *v)
{
	((float*)v)[0] = ReadFloat();
	((float*)v)[1] = ReadFloat();
	((float*)v)[2] = ReadFloat();
}

// write a single character (1 byte)
void CFile::WriteChar(char c)
{
	if (Binary)	int r=_write(handle,&c,1);
	else		WriteStr(i2s(c));
}

// write a single character (1 byte)
void CFile::WriteByte(unsigned char c)
{
	if (Binary)	int r=_write(handle,&c,1);
	else		WriteStr(i2s(c));
}

// write a word (2 bytes)
void CFile::WriteWord(unsigned short w)
{
	/*char c=(w/256)%256;
	write(handle,&c,1);
	c=w%256;
	write(handle,&c,1);*/
	if (Binary)	int r=_write(handle,&w,2);
	else		WriteStr(i2s(w));
}

// write an integer (4 bytes)
void _cdecl CFile::WriteInt(int in)
{
	if (Binary)	int r=_write(handle,&in,4);
	else		WriteStr(i2s(in));
}

// write a float (4 bytes)
void CFile::WriteFloat(float f)
{
	if (Binary)	int r=_write(handle,&f,4);
	else		WriteStr(f2s(f,FloatDecimals));
}

// write a boolean (1 byte)
void _cdecl CFile::WriteBool(bool b)
{
	int r;
	if (b)	r=_write(handle,"1",1);
	else	r=_write(handle,"0",1);
	if (!Binary)	r=_write(handle,"\n",1);
}

// write a string
//   text mode:   complete rest of this line
//   binary mode: length word, then string
void CFile::WriteStr(const string &str)
{
	if (Binary)
		WriteWord(str.num);
	if (str.num > 0)
		int r = _write(handle, str.data, str.num);
	if (!Binary)
		int r = _write(handle, "\n", 1);
}

// write a comment line
void CFile::WriteComment(const string &str)
{
#ifdef FILE_COMMENTS_DEBUG
	msg_write("comment: " + str);
#endif
	if (!Binary)
		WriteStr(str);
}

void CFile::WriteVector(const void *v)
{
	WriteFloat(((float*)v)[0]);
	WriteFloat(((float*)v)[1]);
	WriteFloat(((float*)v)[2]);
}


void CFile::Int(int &i)
{
	if (Reading)
		i = ReadInt();
	else
		WriteInt(i);
}

void CFile::Float(float &f)
{
	if (Reading)
		f = ReadFloat();
	else
		WriteFloat(f);
}

void CFile::Bool(bool &b)
{
	if (Reading)
		b = ReadBool();
	else
		WriteBool(b);
}

void CFile::String(string &str)
{
	if (Reading)
		str = ReadStr();
	else
		WriteStr(str);
}

void CFile::Vector(float *v)
{
	if (Reading)
		ReadVector(v);
	else
		WriteVector(v);
}

struct sStructFormatItem
{
	int offset, size, type, num, total_size;
};

struct sStructFormat
{
	const char *format;
	Array<sStructFormatItem> item_t;
	Array<sStructFormatItem> item_b;
	int size;
};

Array<sStructFormat> StructFormat;

void add_sfi(Array<sStructFormatItem> &it, int offset, int size, int type, int num, int total_size)
{
	sStructFormatItem ii = {offset, size, type, num, total_size};
	it.add(ii);
}

int get_sfi_num(const char *str)
{
	if ((*str >= '1') && (*str <= '9')){
		int n = 0;
		for (int i=0;i<7;i++){
			if ((str[i] >= '1') && (str[i] <= '9'))
				n = n*10 + (str[i] - 48);
			else
				return n;
		}
	}
	return 1;
}

sStructFormat *get_format(const char *format)
{
	for (int i=0;i<StructFormat.num;i++)
		if (StructFormat[i].format == format)
			return &StructFormat[i];
	sStructFormat f;
	f.format = format;
	f.size = 0;
	int l = strlen(format);
	int offset = 0;
	for (int i=0;i<l;i++){
		if (format[i] == 'C'){
			//add_sfi(f.item_t, );
		}else if (format[i] == 'i'){
			int num = get_sfi_num(&format[i + 1]);
			add_sfi(f.item_t, offset, 4, 1, num, 4 * num);
			//add_sfi(f.item_b, offset, 4, 1, num, 4 * num);
			offset += 4 * num;
		}else if (format[i] == 'f'){
			int num = get_sfi_num(&format[i + 1]);
			add_sfi(f.item_t, offset, 4, 2, num, 4 * num);
			//add_sfi(f.item_b, offset, 4, 2, num, 4 * num);
			offset += 4 * num;
		}else if (format[i] == 'b'){
			int num = get_sfi_num(&format[i + 1]);
			add_sfi(f.item_t, offset, 1, 3, num, 1 * num);
			//add_sfi(f.item_b, offset, 1, 3, num, 1 * num);
			offset += 1 * num;
		}
	}
	f.size = offset;
	StructFormat.add(f);
	return &StructFormat.back();
}

void CFile::Struct(const char *format, void *data)
{
	sStructFormat *f = get_format(format);
	if (Reading){
		if (Binary){
			ReadBuffer(data, f->size);
		}else{
			char *cdata = (char*)data;
			for (int i=0;i<f->item_t.num;i++){
				int n = f->item_t[i].num;
				if (f->item_t[i].type == 0)
					ReadComment();
				else if (f->item_t[i].type == 1)
					for (int j=0;j<n;j++)
						*(int*)&cdata[f->item_t[i].offset + j * 4] = ReadInt();
				else if (f->item_t[i].type == 2)
					for (int j=0;j<n;j++)
						*(float*)&cdata[f->item_t[i].offset + j * 4] = ReadFloat();
				else if (f->item_t[i].type == 3)
					for (int j=0;j<n;j++)
						*(bool*)&cdata[f->item_t[i].offset + j] = ReadBool();
			}
		}
	}else{
		if (Binary){
			WriteBuffer(data, f->size);
		}else{
			char *cdata = (char*)data;
			for (int i=0;i<f->item_t.num;i++){
				int n = f->item_t[i].num;
				if (f->item_t[i].type == 0)
					msg_todo("CFile::Struct can't write comments");
				else if (f->item_t[i].type == 1)
					for (int j=0;j<n;j++)
						WriteInt(*(int*)&cdata[f->item_t[i].offset + j * 4]);
				else if (f->item_t[i].type == 2)
					for (int j=0;j<n;j++)
						WriteFloat(*(float*)&cdata[f->item_t[i].offset + j * 4]);
				else if (f->item_t[i].type == 3)
					for (int j=0;j<n;j++)
						WriteBool(*(bool*)&cdata[f->item_t[i].offset + j]);
			}
		}
		//msg_todo("CFile::Struct writing mode");
	}
}

void CFile::StructN(const char *format, int &num, void *data, int shift)
{
	if (Reading)
		num = ReadInt();
	else
		WriteInt(num);
	for (int i=0;i<num;i++)
		Struct(format, &((char*)data)[shift * i]);
}

// insert some white spaces
void CFile::ShiftRight(int s)
{
	int r;
#ifdef StructuredShifts
	for (int i=0;i<s-1;i++)
		r=_write(handle," |\t",3);
	if (s>0)
		r=_write(handle," >-\t",4);
#else
	for (int i=0;i<s;i++)
		r=_write(handle,"\t",1);
#endif
}

