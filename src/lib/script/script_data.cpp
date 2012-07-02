/*----------------------------------------------------------------------------*\
| Script Data                                                                  |
| -> "standart library" for the scripting system                               |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2010.07.07 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#include <algorithm>
#include <string.h>

#ifdef WIN32
	#include "windows.h"
#endif
#include "script.h"
#include "script_data_common.h"
#include "../00_config.h"

string ScriptDataVersion = "0.10.1.0";


#ifdef _X_USE_HUI_
#include "../hui/hui.h"
#endif




int ScriptStackSize = SCRIPT_DEFAULT_STACK_SIZE;



//------------------------------------------------------------------------------------------------//
//                                             types                                              //
//------------------------------------------------------------------------------------------------//

sType *TypeUnknown;
sType *TypeReg32;
sType *TypeReg16;
sType *TypeReg8;
sType *TypeVoid;
sType *TypePointer;
sType *TypeClass;
sType *TypeBool;
sType *TypeInt;
sType *TypeFloat;
sType *TypeChar;
sType *TypeString;
sType *TypeCString;
sType *TypeSuperArray;

sType *TypeVector;
sType *TypeRect;
sType *TypeColor;
sType *TypeQuaternion;
 // internal:
sType *TypePointerPs;
sType *TypePointerList;
sType *TypeBoolList;
sType *TypeIntPs;
sType *TypeIntList;
sType *TypeIntArray;
sType *TypeFloatPs;
sType *TypeFloatList;
sType *TypeFloatArray;
sType *TypeFloatArrayP;
sType *TypeComplex;
sType *TypeComplexList;
sType *TypeStringList;
sType *TypeVectorArray;
sType *TypeVectorArrayP;
sType *TypeVectorList;
sType *TypeMatrix;
sType *TypePlane;
sType *TypePlaneList;
sType *TypeMatrix3;
sType *TypeDate;
sType *TypeImage;


Array<sPackage> Package;
sPackage *cur_package = NULL;
int cur_package_index = -1;


void set_cur_package(const string &name)
{
	cur_package_index = Package.num;
	sPackage p;
	p.name = name;
	Package.add(p);
	cur_package = &Package.back();
}

Array<sType*> PreType;
sType *add_type(const string &name, int size, TypeFlag flag)
{
	msg_db_r("add_type", 4);
	sType *t = new sType;
	t->Name = name;
	t->Size = size;
	if ((flag & FLAG_CALL_BY_VALUE) > 0)
		t->ForceCallByValue = true;
	PreType.add(t);
	if (cur_package)
		cur_package->type.add(t);
	msg_db_l(4);
	return t;
}
sType *add_type_p(const string &name, sType *sub_type, TypeFlag flag)
{
	msg_db_r("add_type_p", 4);
	sType *t = new sType;
	t->Name = name;
	t->Size = PointerSize;
	t->IsPointer = true;
	if ((flag & FLAG_SILENT) > 0)
		t->IsSilent = true;
	t->SubType = sub_type;
	PreType.add(t);
	if (cur_package)
		cur_package->type.add(t);
	msg_db_l(4);
	return t;
}
sType *add_type_a(const string &name, sType *sub_type, int array_length)
{
	msg_db_r("add_type_a", 4);
	sType *t = new sType;
	t->Name = name;
	t->SubType = sub_type;
	if (array_length < 0){
		// super array
		t->Size = SuperArraySize;
		t->IsSuperArray = true;
		//script_make_super_array(t); // do it later !!!
	}else{
		// standard array
		t->Size = sub_type->Size * array_length;
		t->IsArray = true;
		t->ArrayLength = array_length;
	}
	PreType.add(t);
	if (cur_package)
		cur_package->type.add(t);
	msg_db_l(4);
	return t;
}

sType *ScriptGetPreType(const string &name)
{
	for (int i=0;i<PreType.num;i++)
		if (name == PreType[i]->Name)
			return PreType[i];
	return TypeUnknown;
}

//------------------------------------------------------------------------------------------------//
//                                           operators                                            //
//------------------------------------------------------------------------------------------------//

//   without type information ("primitive")
int NumPrimitiveOperators = NUM_PRIMITIVE_OPERATORS;

sPrimitiveOperator PrimitiveOperator[NUM_PRIMITIVE_OPERATORS]={
	{"=",	OperatorAssign,			true,	1,	"__assign__"},
	{"+",	OperatorAdd,			false,	11,	"__add__"},
	{"-",	OperatorSubtract,		false,	11,	"__sub__"},
	{"*",	OperatorMultiply,		false,	12,	"__mul__"},
	{"/",	OperatorDivide,			false,	12,	"__div__"},
	{"+=",	OperatorAddS,			true,	1,	"__iadd__"},
	{"-=",	OperatorSubtractS,		true,	1,	"__isub__"},
	{"*=",	OperatorMultiplyS,		true,	1,	"__imul__"},
	{"/=",	OperatorDivideS,		true,	1,	"__idiv__"},
	{"==",	OperatorEqual,			false,	8,	"__eq__"},
	{"!=",	OperatorNotEqual,		false,	8,	"__ne__"},
	{"!",	OperatorNegate,			false,	2,	"__not__"},
	{"<",	OperatorSmaller,		false,	9,	"__lt__"},
	{">",	OperatorGreater,		false,	9,	"__gt__"},
	{"<=",	OperatorSmallerEqual,	false,	9,	"__le__"},
	{">=",	OperatorGreaterEqual,	false,	9,	"__ge__"},
	{"and",	OperatorAnd,			false,	4,	"__and__"},
	{"or",	OperatorOr,				false,	3,	"__or__"},
	{"%",	OperatorModulo,			false,	12,	"__mod__"},
	{"&",	OperatorBitAnd,			false,	7,	"__bitand__"},
	{"|",	OperatorBitOr,			false,	5,	"__bitor__"},
	{"<<",	OperatorShiftLeft,		false,	10,	"__lshift__"},
	{">>",	OperatorShiftRight,		false,	10,	"__rshift__"},
	{"++",	OperatorIncrease,		true,	2,	"__inc__"},
	{"--",	OperatorDecrease,		true,	2,	"__dec__"}
// Level = 15 - (official C-operator priority)
// priority from "C als erste Programmiersprache", page 552
};

//   with type information

Array<sPreOperator> PreOperator;
int add_operator(int primitive_op, sType *return_type, sType *param_type1, sType *param_type2, void *func = NULL)
{
	msg_db_r("add_op", 4);
	sPreOperator o;
	o.PrimitiveID = primitive_op;
	o.ReturnType = return_type;
	o.ParamType1 = param_type1;
	o.ParamType2 = param_type2;
	o.Func = func;
	PreOperator.add(o);
	msg_db_l(4);
	return PreOperator.num - 1;
}


//------------------------------------------------------------------------------------------------//
//                                     classes & elements                                         //
//------------------------------------------------------------------------------------------------//



sType *cur_class;
sClassFunction *cur_class_func = NULL;

void add_class(sType *root_type)//, CPreScript *ps = NULL)
{
	msg_db_r("add_class", 4);
	cur_class = root_type;
	msg_db_l(4);
}

void class_add_element(const string &name, sType *type, int offset)
{
	msg_db_r("add_class_el", 4);
	sClassElement e;
	e.Name = name;
	e.Type = type;
	e.Offset = offset;
	cur_class->Element.add(e);
	msg_db_l(4);
}

int add_func(const string &name, sType *return_type, void *func, bool is_class);

void class_add_func(const string &name, sType *return_type, void *func)
{
	msg_db_r("add_class_func", 4);
	string tname = cur_class->Name;
	if (tname[0] == '-')
		for (int i=0;i<PreType.num;i++)
			if ((PreType[i]->IsPointer) && (PreType[i]->SubType == cur_class))
				tname = PreType[i]->Name;
	int cmd = add_func(tname + "." + name, return_type, func, true);
	sClassFunction f;
	f.Name = name;
	f.Kind = KindCompilerFunction;
	f.Nr = cmd;
	f.ReturnType = return_type;
	cur_class->Function.add(f);
	cur_class_func = &cur_class->Function.back();
	msg_db_l(4);
}


//------------------------------------------------------------------------------------------------//
//                                           constants                                            //
//------------------------------------------------------------------------------------------------//

Array<sPreConstant> PreConstant;
void add_const(const string &name, sType *type, void *value)
{
	msg_db_r("add_const", 4);
	sPreConstant c;
	c.Name = name;
	c.Type = type;
	c.Value = value;
	c.package = cur_package_index;
	PreConstant.add(c);
	msg_db_l(4);
}

//------------------------------------------------------------------------------------------------//
//                                    environmental variables                                     //
//------------------------------------------------------------------------------------------------//

Array<sPreExternalVar> PreExternalVar;

void add_ext_var(const string &name, sType *type, void *var)
{
	sPreExternalVar v;
	v.Name = name;
	v.Type = type;
	v.Pointer = var;
	v.IsSemiExternal = false;
	v.package = cur_package_index;
	PreExternalVar.add(v);
};

//------------------------------------------------------------------------------------------------//
//                                      compiler functions                                        //
//------------------------------------------------------------------------------------------------//



#ifndef FILE_OS_WINDOWS
	//#define _cdecl
	#include <stdlib.h>
#endif

//void _cdecl _stringout(char *str){	msg_write(string("StringOut: ",str));	}
void _cdecl _cstringout(char *str){	msg_write(str);	}
void _cdecl _stringout(string &str){	msg_write(str);	}
int _cdecl _Float2Int(float f){	return (int)f;	}
string _cdecl ff2s(complex &x){	return x.str();	}
string _cdecl fff2s(vector &x){	return x.str();	}
string _cdecl ffff2s(quaternion &x){	return x.str();	}


void *f_cp = (void*)1; // for fake (compiler-) functions


Array<sPreCommand> PreCommand;

int cur_func;

int add_func(const string &name, sType *return_type, void *func, bool is_class)
{
	sPreCommand c;
	c.Name = name;
	c.ReturnType = return_type;
	c.Func = func;
	c.IsSpecial = false;
	c.IsClassFunction = is_class;
	c.IsSemiExternal = false;
	c.package = cur_package_index;
	if (PreCommand.num < NUM_INTERN_PRE_COMMANDS)
		PreCommand.resize(NUM_INTERN_PRE_COMMANDS);
	PreCommand.add(c);
	cur_func = PreCommand.num - 1;
	return cur_func;
}

int add_func_special(const string &name, sType *return_type, int index)
{
	sPreCommand c;
	c.Name = name;
	c.ReturnType = return_type;
	c.Func = NULL;
	c.IsSpecial = true;
	c.IsClassFunction = false;
	c.IsSemiExternal = false;
	c.package = cur_package_index;
	if (PreCommand.num < NUM_INTERN_PRE_COMMANDS)
		PreCommand.resize(NUM_INTERN_PRE_COMMANDS);
	PreCommand[index] = c;
	cur_func = index;
	cur_class_func = NULL;
	return cur_func;
}

void func_add_param(const string &name, sType *type)
{
	sPreCommandParam p;
	p.Name = name;
	p.Type = type;
	PreCommand[cur_func].Param.add(p);
	if (cur_class_func)
		cur_class_func->ParamType.add(type);
}

void CSuperArray::init_by_type(sType *t)
{	init(t->Size);	}

/*string super_array_add_str(string *a, string *b)
{
	string r;
	//r.init(1); // done by kaba-constructors for temp variables
	r.assign(a);
	r.append(b);
	return r;
}

void super_array_assign_str_cstr(string *a, char *b)
{
	int l = strlen(b);
	a->resize(l);
	memcpy((char*)a->data, b, l);
}

void super_array_add_str_cstr(string *a, char *b)
{
	int n_old = a->num;
	int l = strlen(b);
	a->resize(a->num + l);
	memcpy(&((char*)a->data)[n_old], b, l);
}*/

bool type_is_simple_class(sType *t)
{
	if (!t->UsesCallByReference())
		return true;
	/*if (t->IsArray)
		return false;*/
	if (t->IsSuperArray)
		return false;
	if (t->GetFunc("__init__") >= 0)
		return false;
	if (t->GetFunc("__delete__") >= 0)
		return false;
	if (t->GetFunc("__assign__") >= 0)
		return false;
	foreach(t->Element, e)
		if (!type_is_simple_class(e.Type))
			return false;
	return true;
}

void script_make_super_array(sType *t, CPreScript *ps)
{
	msg_db_r("make_super_array", 4);
	add_class(t);
		class_add_element("num", TypeInt, PointerSize);

		// always usable operations
		class_add_func("swap", TypeVoid, mf((tmf)&CSuperArray::swap));
			func_add_param("i1",		TypeInt);
			func_add_param("i2",		TypeInt);
		class_add_func("iterate", TypeBool, mf((tmf)&CSuperArray::iterate));
			func_add_param("pointer",		TypePointerPs);
		class_add_func("iterate_back", TypeBool, mf((tmf)&CSuperArray::iterate_back));
			func_add_param("pointer",		TypePointerPs);
		class_add_func("index", TypeInt, mf((tmf)&CSuperArray::index));
			func_add_param("pointer",		TypePointer);
		class_add_func("subarray", t, mf((tmf)&CSuperArray::ref_subarray));
			func_add_param("start",		TypeInt);
			func_add_param("num",		TypeInt);

		if (type_is_simple_class(t->SubType)){
			if (!t->SubType->UsesCallByReference()){
				if (t->SubType->Size == 4){
					class_add_func("__init__",	TypeVoid, mf((tmf)&Array<int>::__init__));
					class_add_func("add", TypeVoid, mf((tmf)&CSuperArray::append_4_single));
						func_add_param("x",		t->SubType);
				}else if (t->SubType->Size == 1){
					class_add_func("__init__",	TypeVoid, mf((tmf)&Array<char>::__init__));
					class_add_func("add", TypeVoid, mf((tmf)&CSuperArray::append_1_single));
						func_add_param("x",		t->SubType);
				}
			}else{
				class_add_func("add", TypeVoid, mf((tmf)&CSuperArray::append_single));
					func_add_param("x",		t->SubType);
			}
			class_add_func("__delete__",	TypeVoid, mf((tmf)&CSuperArray::clear));
			class_add_func("clear", TypeVoid, mf((tmf)&CSuperArray::clear));
			class_add_func("__assign__", TypeVoid, mf((tmf)&CSuperArray::assign));
				func_add_param("other",		t);
			class_add_func("remove", TypeVoid, mf((tmf)&CSuperArray::delete_single));
				func_add_param("index",		TypeInt);
			class_add_func("removep", TypeVoid, mf((tmf)&CSuperArray::delete_single_by_pointer));
				func_add_param("pointer",		TypePointer);
			class_add_func("resize", TypeVoid, mf((tmf)&CSuperArray::resize));
				func_add_param("num",		TypeInt);
			class_add_func("ensure_size", TypeVoid, mf((tmf)&CSuperArray::ensure_size));
				func_add_param("num",		TypeInt);
		}

		// low level operations
		class_add_func("__mem_init__", TypeVoid, mf((tmf)&CSuperArray::init));
			func_add_param("element_size",		TypeInt);
		class_add_func("__mem_clear__", TypeVoid, mf((tmf)&CSuperArray::clear));
		class_add_func("__mem_resize__", TypeVoid, mf((tmf)&CSuperArray::resize));
			func_add_param("size",		TypeInt);
		class_add_func("__mem_remove__", TypeVoid, mf((tmf)&CSuperArray::delete_single));
			func_add_param("index",		TypeInt);
	msg_db_l(4);
}


// automatic type casting

#define MAX_TYPE_CAST_BUFFER	32768
char type_cast_buffer[MAX_TYPE_CAST_BUFFER];
int type_cast_buffer_size = 0;

inline char *get_type_cast_buf(int size)
{
	char *str = &type_cast_buffer[type_cast_buffer_size];
	type_cast_buffer_size += size;
	if (type_cast_buffer_size >= MAX_TYPE_CAST_BUFFER){
		msg_error("Script: type_cast_buffer overflow");
		type_cast_buffer_size = 0;
		str = type_cast_buffer;
	}
	return str;
}


char CastTemp[256];
char *CastFloat2Int(float *f)
{
	*(int*)&CastTemp[0]=int(*f);
	return &CastTemp[0];
}
char *CastInt2Float(int *i)
{
	*(float*)&CastTemp[0]=float(*i);
	return &CastTemp[0];
}
char *CastInt2Char(int *i)
{
	*(char*)&CastTemp[0]=char(*i);
	return &CastTemp[0];
}
char *CastChar2Int(char *c)
{
	*(int*)&CastTemp[0]=int(*c);
	return &CastTemp[0];
}
char *CastPointer2Bool(void **p)
{
	*(bool*)&CastTemp[0]=( (*p) != NULL );
	return &CastTemp[0];
}
char *CastInt2StringP(int *i)
{
	string s = i2s(*i);
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}
char *CastFloat2StringP(float *f)
{
	string s = f2sf(*f);
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}
char *CastBool2StringP(bool *b)
{
	string s = b2s(*b);
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	type_cast_buffer_size += strlen(str) + 1;
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}
char *CastPointer2StringP(void *p)
{
	string s = p2s(p);
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}
char *CastVector2StringP(vector *v)
{
	string s = v->str();
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}
char *CastFFFF2StringP(quaternion *q)
{
	string s = q->str();
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}
char *CastComplex2StringP(complex *z)
{
	string s = z->str();
	char *str = get_type_cast_buf(s.num + 1);
	memcpy(str, s.data, s.num);
	*(char**)&CastTemp[0] = str; // save the return address in CastTemp
	return &CastTemp[0];
}

Array<sTypeCast> TypeCast;
void add_type_cast(int penalty, sType *source, sType *dest, const string &cmd, void *func)
{
	sTypeCast c;
	c.Penalty = penalty;
	c.Command = -1;
	for (int i=0;i<PreCommand.num;i++)
		if (PreCommand[i].Name == cmd){
			c.Command = i;
			break;
		}
	if (c.Command < 0){
#ifdef _X_USE_HUI_
		HuiErrorBox(NULL, "", "add_type_cast (ScriptInit): " + string(cmd) + " not found");
		HuiRaiseError("add_type_cast (ScriptInit): " + string(cmd) + " not found");
#else
		msg_error("add_type_cast (ScriptInit): " + string(cmd) + " not found"));
		exit(1);
#endif
	}
	c.Source = source;
	c.Dest = dest;
	c.Func = (t_cast_func*) func;
	TypeCast.add(c);
}


class StringList : public Array<string>
{
public:
	void assign(StringList &s){	*this = s;	}
	string join(const string &glue)
	{
		string r;
		foreachi(*this, s, i){
			if (i > 0)
				r += glue;
			r += s;
		}
		return r;
	}
};

class IntClass
{
	int i;
public:
	string str(){	return i2s(i);	}
};

class FloatClass
{
	float f;
public:
	string str(){	return f2s(f, 6);	}
};

class BoolClass
{
	bool b;
public:
	string str(){	return b2s(b);	}
};

class CharClass
{
	char c;
public:
	string str(){	string r;	r.add(c);	return r;	}
};

class PointerClass
{
	void *p;
public:
	string str(){	return p2s(p);	}
};

void SIAddPackageBase()
{
	msg_db_r("SIAddPackageBase", 3);

	set_cur_package("base");

	// internal
	TypeUnknown			= add_type  ("-\?\?\?-",	0); // should not appear anywhere....or else we're screwed up!
	TypeReg32			= add_type  ("-reg32-",		PointerSize);
	TypeReg16			= add_type  ("-reg16-",		PointerSize);
	TypeReg8			= add_type  ("-reg8-",		PointerSize);
	TypeClass			= add_type  ("-class-",	0); // substitute for all class types

	// "real"
	TypeVoid			= add_type  ("void",		0);
	TypeSuperArray		= add_type_a("void[]",		TypeVoid, -1); // substitute for all super arrays
	TypePointer			= add_type_p("void*",		TypeVoid, FLAG_CALL_BY_VALUE); // substitute for all pointer types
	TypePointerPs		= add_type_p("void*&",		TypePointer, FLAG_SILENT);
	TypePointerList		= add_type_a("void*[]",		TypePointer, -1);
	TypeBool			= add_type  ("bool",		sizeof(bool), FLAG_CALL_BY_VALUE);
	TypeBoolList		= add_type_a("bool[]",		TypeBool, -11);
	TypeInt				= add_type  ("int",			sizeof(int), FLAG_CALL_BY_VALUE);
	TypeIntPs			= add_type_p("int&",		TypeInt, FLAG_SILENT);
	TypeIntList			= add_type_a("int[]",		TypeInt, -1);
	TypeIntArray		= add_type_a("int[?]",		TypeInt, 1);
	TypeFloat			= add_type  ("float",		sizeof(float), FLAG_CALL_BY_VALUE);
	TypeFloatPs			= add_type_p("float&",		TypeFloat, FLAG_SILENT);
	TypeFloatArray		= add_type_a("float[?]",	TypeFloat, 1);
	TypeFloatArrayP		= add_type_p("float[?]*",	TypeFloatArray);
	TypeFloatList		= add_type_a("float[]",		TypeFloat, -1);
	TypeChar			= add_type  ("char",		sizeof(char), FLAG_CALL_BY_VALUE);
	TypeCString			= add_type_a("cstring",		TypeChar, 256);	// cstring := char[256]
	TypeString			= add_type_a("string",		TypeChar, -1);	// string := char[]
	TypeStringList		= add_type_a("string[]",	TypeString, -1);

	
	add_class(TypeInt);
		class_add_func("str", TypeString, mf((tmf)&IntClass::str));
	add_class(TypeFloat);
		class_add_func("str", TypeString, mf((tmf)&FloatClass::str));
	add_class(TypeBool);
		class_add_func("str", TypeString, mf((tmf)&BoolClass::str));
	add_class(TypeChar);
		class_add_func("str", TypeString, mf((tmf)&CharClass::str));
	add_class(TypePointer);
		class_add_func("str", TypeString, mf((tmf)&PointerClass::str));
	
	add_class(TypeString);
		class_add_func("__iadd__", TypeVoid, mf((tmf)&string::operator+=));
			func_add_param("x",		TypeString);
		class_add_func("__add__", TypeString, mf((tmf)&string::operator+));
			func_add_param("x",		TypeString);
		class_add_func("__eq__", TypeBool, mf((tmf)&string::operator==));
			func_add_param("x",		TypeString);
		class_add_func("__ne__", TypeBool, mf((tmf)&string::operator!=));
			func_add_param("x",		TypeString);
		class_add_func("__lt__", TypeBool, mf((tmf)&string::operator<));
			func_add_param("x",		TypeString);
		class_add_func("__gt__", TypeBool, mf((tmf)&string::operator>));
			func_add_param("x",		TypeString);
		class_add_func("__le__", TypeBool, mf((tmf)&string::operator<=));
			func_add_param("x",		TypeString);
		class_add_func("__ge__", TypeBool, mf((tmf)&string::operator>=));
			func_add_param("x",		TypeString);
		class_add_func("substr", TypeString, mf((tmf)&string::substr));
			func_add_param("start",		TypeInt);
			func_add_param("length",	TypeInt);
		class_add_func("find", TypeInt, mf((tmf)&string::find));
			func_add_param("str",		TypeString);
			func_add_param("start",		TypeInt);
		class_add_func("compare", TypeInt, mf((tmf)&string::compare));
			func_add_param("str",		TypeString);
		class_add_func("icompare", TypeInt, mf((tmf)&string::icompare));
			func_add_param("str",		TypeString);
		class_add_func("replace", TypeVoid, mf((tmf)&string::replace));
			func_add_param("sub",		TypeString);
			func_add_param("by",		TypeString);
		class_add_func("explode", TypeStringList, mf((tmf)&string::explode));
			func_add_param("str",		TypeString);
		class_add_func("lower", TypeString, mf((tmf)&string::lower));
		class_add_func("upper", TypeString, mf((tmf)&string::upper));

	add_class(TypeStringList);
		class_add_func("__init__",	TypeVoid, mf((tmf)&StringList::__init__));
		class_add_func("__delete__",	TypeVoid, mf((tmf)&StringList::clear));
		class_add_func("add", TypeVoid, mf((tmf)&StringList::add));
			func_add_param("x",		TypeString);
		class_add_func("clear", TypeVoid, mf((tmf)&StringList::clear));
		class_add_func("remove", TypeVoid, mf((tmf)&StringList::erase));
			func_add_param("index",		TypeInt);
		class_add_func("resize", TypeVoid, mf((tmf)&StringList::resize));
			func_add_param("num",		TypeInt);
		class_add_func("__assign__",	TypeVoid, mf((tmf)&StringList::assign));
			func_add_param("other",		TypeStringList);
		class_add_func("join", TypeString, mf((tmf)&StringList::join));
			func_add_param("glue",		TypeString);


	add_const("nil", TypePointer, NULL);
	// bool
	add_const("false", TypeBool, (void*)false);
	add_const("true",  TypeBool, (void*)true);
	
	msg_db_l(3);
}


void SIAddBasicCommands()
{
	msg_db_r("SIAddBasicCommands", 3);

/*
	CommandReturn,
	CommandIf,
	CommandIfElse,
	CommandWhile,
	CommandFor,
	CommandBreak,
	CommandContinue,
	CommandSizeof,
	CommandWait,
	CommandWaitRT,
	CommandWaitOneFrame,
	CommandFloatToInt,
	CommandIntToFloat,
	CommandIntToChar,
	CommandCharToInt,
	CommandPointerToBool,
	CommandComplexSet,
	CommandVectorSet,
	CommandRectSet,
	CommandColorSet,
	CommandAsm,
*/


// "intern" functions
	add_func_special("return",		TypeVoid,	CommandReturn);
		func_add_param("return_value",	TypeVoid); // return: ParamType will be defined by the parser!
	add_func_special("-if-",		TypeVoid,	CommandIf);
		func_add_param("b",	TypeBool);
	add_func_special("-if/else-",	TypeVoid,	CommandIfElse);
		func_add_param("b",	TypeBool);
	add_func_special("-while-",		TypeVoid,	CommandWhile);
		func_add_param("b",	TypeBool);
	add_func_special("-for-",		TypeVoid,	CommandFor);
		func_add_param("b",	TypeBool); // internally like a while-loop... but a bit different...
	add_func_special("-break-",		TypeVoid,	CommandBreak);
	add_func_special("-continue-",	TypeVoid,	CommandContinue);
	add_func_special("sizeof",		TypeInt,	CommandSizeof);
		func_add_param("type",	TypeVoid);
	
	add_func_special("wait",		TypeVoid,	CommandWait);
		func_add_param("time",	TypeFloat);
	add_func_special("wait_rt",		TypeVoid,	CommandWaitRT);
		func_add_param("time",	TypeFloat);
	add_func_special("wait_of",		TypeVoid,	CommandWaitOneFrame);
//	add_func_special("f2i",			TypeInt,	(void*)&_Float2Int);
	add_func_special("f2i",			TypeInt,	CommandFloatToInt);    // sometimes causes floating point exceptions...
		func_add_param("f",		TypeFloat);
	add_func_special("i2f",			TypeFloat,	CommandIntToFloat);
		func_add_param("i",		TypeInt);
	add_func_special("i2c",			TypeChar,	CommandIntToChar);
		func_add_param("i",		TypeInt);
	add_func_special("c2i",			TypeInt,	CommandCharToInt);
		func_add_param("c",		TypeChar);
	add_func_special("p2b",			TypeBool,	CommandPointerToBool);
		func_add_param("p",		TypePointer);
	add_func_special("-asm-",		TypeVoid,	CommandAsm);
	
	msg_db_l(3);
}



void op_int_add(int &r, int &a, int &b)
{	r = a + b;	}
void op_int_sub(int &r, int &a, int &b)
{	r = a - b;	}
void op_int_mul(int &r, int &a, int &b)
{	r = a * b;	}
void op_int_div(int &r, int &a, int &b)
{	r = a / b;	}
void op_int_mod(int &r, int &a, int &b)
{	r = a % b;	}
void op_int_shr(int &r, int &a, int &b)
{	r = a >> b;	}
void op_int_shl(int &r, int &a, int &b)
{	r = a << b;	}
void op_float_add(float &r, float &a, float &b)
{	r = a + b;	}
void op_float_sub(float &r, float &a, float &b)
{	r = a - b;	}
void op_float_mul(float &r, float &a, float &b)
{	r = a * b;	}
void op_float_div(float &r, float &a, float &b)
{	r = a / b;	}

void SIAddOperators()
{
	msg_db_r("SIAddOperators", 3);
	

	// same order as in .h file...
	add_operator(OperatorAssign,		TypeVoid,		TypePointer,	TypePointer);
	add_operator(OperatorEqual,			TypeBool,		TypePointer,	TypePointer);
	add_operator(OperatorNotEqual,		TypeBool,		TypePointer,	TypePointer);
	add_operator(OperatorAssign,		TypeVoid,		TypeChar,		TypeChar);
	add_operator(OperatorEqual,			TypeBool,		TypeChar,		TypeChar);
	add_operator(OperatorNotEqual,		TypeBool,		TypeChar,		TypeChar);
	add_operator(OperatorAdd,			TypeChar,		TypeChar,		TypeChar);
	add_operator(OperatorSubtractS,		TypeChar,		TypeChar,		TypeChar);
	add_operator(OperatorAddS,			TypeChar,		TypeChar,		TypeChar);
	add_operator(OperatorSubtract,		TypeChar,		TypeChar,		TypeChar);
	add_operator(OperatorBitAnd,		TypeChar,		TypeChar,		TypeChar);
	add_operator(OperatorBitOr,			TypeChar,		TypeChar,		TypeChar);
	add_operator(OperatorSubtract,		TypeChar,		TypeVoid,		TypeChar);
	add_operator(OperatorAssign,		TypeVoid,		TypeBool,		TypeBool);
	add_operator(OperatorEqual,			TypeBool,		TypeBool,		TypeBool);
	add_operator(OperatorNotEqual,		TypeBool,		TypeBool,		TypeBool);
	add_operator(OperatorGreater,		TypeBool,		TypeBool,		TypeBool); // ???????? char? FIXME
	add_operator(OperatorGreaterEqual,	TypeBool,		TypeBool,		TypeBool);
	add_operator(OperatorSmaller,		TypeBool,		TypeBool,		TypeBool);
	add_operator(OperatorSmallerEqual,	TypeBool,		TypeBool,		TypeBool);
	add_operator(OperatorAnd,			TypeBool,		TypeBool,		TypeBool);
	add_operator(OperatorOr,			TypeBool,		TypeBool,		TypeBool);
	add_operator(OperatorNegate,		TypeBool,		TypeVoid,		TypeBool);	
	add_operator(OperatorAssign,		TypeVoid,		TypeInt,		TypeInt);
	add_operator(OperatorAdd,			TypeInt,		TypeInt,		TypeInt,	(void*)op_int_add);
	add_operator(OperatorSubtract,		TypeInt,		TypeInt,		TypeInt,	(void*)op_int_sub);
	add_operator(OperatorMultiply,		TypeInt,		TypeInt,		TypeInt,	(void*)op_int_mul);
	add_operator(OperatorDivide,		TypeInt,		TypeInt,		TypeInt,	(void*)op_int_div);
	add_operator(OperatorAddS,			TypeVoid,		TypeInt,		TypeInt);
	add_operator(OperatorSubtractS,		TypeVoid,		TypeInt,		TypeInt);
	add_operator(OperatorMultiplyS,		TypeVoid,		TypeInt,		TypeInt);
	add_operator(OperatorDivideS,		TypeVoid,		TypeInt,		TypeInt);
	add_operator(OperatorModulo,		TypeInt,		TypeInt,		TypeInt,	(void*)op_int_mod);
	add_operator(OperatorEqual,			TypeBool,		TypeInt,		TypeInt);
	add_operator(OperatorNotEqual,		TypeBool,		TypeInt,		TypeInt);
	add_operator(OperatorGreater,		TypeBool,		TypeInt,		TypeInt);
	add_operator(OperatorGreaterEqual,	TypeBool,		TypeInt,		TypeInt);
	add_operator(OperatorSmaller,		TypeBool,		TypeInt,		TypeInt);
	add_operator(OperatorSmallerEqual,	TypeBool,		TypeInt,		TypeInt);
	add_operator(OperatorBitAnd,		TypeInt,		TypeInt,		TypeInt);
	add_operator(OperatorBitOr,			TypeInt,		TypeInt,		TypeInt);
	add_operator(OperatorShiftRight,	TypeInt,		TypeInt,		TypeInt,	(void*)op_int_shr);
	add_operator(OperatorShiftLeft,		TypeInt,		TypeInt,		TypeInt,	(void*)op_int_shl);
	add_operator(OperatorSubtract,		TypeInt,		TypeVoid,		TypeInt);
	add_operator(OperatorIncrease,		TypeVoid,		TypeInt,		TypeVoid);
	add_operator(OperatorDecrease,		TypeVoid,		TypeInt,		TypeVoid);
	add_operator(OperatorAssign,		TypeVoid,		TypeFloat,		TypeFloat);
	add_operator(OperatorAdd,			TypeFloat,		TypeFloat,		TypeFloat,	(void*)op_float_add);
	add_operator(OperatorSubtract,		TypeFloat,		TypeFloat,		TypeFloat,	(void*)op_float_sub);
	add_operator(OperatorMultiply,		TypeFloat,		TypeFloat,		TypeFloat,	(void*)op_float_mul);
	add_operator(OperatorMultiply,		TypeFloat,		TypeFloat,		TypeInt);
	add_operator(OperatorMultiply,		TypeFloat,		TypeInt,		TypeFloat);
	add_operator(OperatorDivide,		TypeFloat,		TypeFloat,		TypeFloat,	(void*)op_float_div);
	add_operator(OperatorAddS,			TypeVoid,		TypeFloat,		TypeFloat);
	add_operator(OperatorSubtractS,		TypeVoid,		TypeFloat,		TypeFloat);
	add_operator(OperatorMultiplyS,		TypeVoid,		TypeFloat,		TypeFloat);
	add_operator(OperatorDivideS,		TypeVoid,		TypeFloat,		TypeFloat);
	add_operator(OperatorEqual,			TypeBool,		TypeFloat,		TypeFloat);
	add_operator(OperatorNotEqual,		TypeBool,		TypeFloat,		TypeFloat);
	add_operator(OperatorGreater,		TypeBool,		TypeFloat,		TypeFloat);
	add_operator(OperatorGreaterEqual,	TypeBool,		TypeFloat,		TypeFloat);
	add_operator(OperatorSmaller,		TypeBool,		TypeFloat,		TypeFloat);
	add_operator(OperatorSmallerEqual,	TypeBool,		TypeFloat,		TypeFloat);
	add_operator(OperatorSubtract,		TypeFloat,		TypeVoid,		TypeFloat);
//	add_operator(OperatorAssign,		TypeVoid,		TypeComplex,	TypeComplex);
	add_operator(OperatorAdd,			TypeComplex,	TypeComplex,	TypeComplex);
	add_operator(OperatorSubtract,		TypeComplex,	TypeComplex,	TypeComplex);
	add_operator(OperatorMultiply,		TypeComplex,	TypeComplex,	TypeComplex);
	add_operator(OperatorMultiply,		TypeComplex,	TypeFloat,		TypeComplex);
	add_operator(OperatorMultiply,		TypeComplex,	TypeComplex,	TypeFloat);
	add_operator(OperatorDivide,		TypeComplex,	TypeComplex,	TypeComplex);
	add_operator(OperatorAddS,			TypeVoid,		TypeComplex,	TypeComplex);
	add_operator(OperatorSubtractS,		TypeVoid,		TypeComplex,	TypeComplex);
	add_operator(OperatorMultiplyS,		TypeVoid,		TypeComplex,	TypeComplex);
	add_operator(OperatorDivideS,		TypeVoid,		TypeComplex,	TypeComplex);
	add_operator(OperatorEqual,			TypeBool,		TypeComplex,	TypeComplex);
	add_operator(OperatorSubtract,		TypeComplex,	TypeVoid,		TypeComplex);
	add_operator(OperatorAssign,		TypeVoid,		TypeCString,	TypeCString);
	add_operator(OperatorAdd,			TypeCString,	TypeCString,	TypeCString);
	add_operator(OperatorAddS,			TypeVoid,		TypeCString,	TypeCString);
	add_operator(OperatorEqual,			TypeBool,		TypeCString,	TypeCString);
	add_operator(OperatorNotEqual,		TypeBool,		TypeCString,	TypeCString);
	add_operator(OperatorAssign,		TypeVoid,		TypeClass,		TypeClass);
	add_operator(OperatorEqual,			TypeBool,		TypeClass,		TypeClass);
	add_operator(OperatorNotEqual,		TypeBool,		TypeClass,		TypeClass);
	add_operator(OperatorAdd,			TypeVector,		TypeVector,		TypeVector);
	add_operator(OperatorSubtract,		TypeVector,		TypeVector,		TypeVector);
	add_operator(OperatorMultiply,		TypeVector,		TypeVector,		TypeVector);
	add_operator(OperatorMultiply,		TypeVector,		TypeVector,		TypeFloat);
	add_operator(OperatorMultiply,		TypeVector,		TypeFloat,		TypeVector);
	add_operator(OperatorDivide,		TypeVector,		TypeVector,		TypeVector);
	add_operator(OperatorDivide,		TypeVector,		TypeVector,		TypeFloat);
	add_operator(OperatorAddS,			TypeVoid,		TypeVector,		TypeVector);
	add_operator(OperatorSubtractS,		TypeVoid,		TypeVector,		TypeVector);
	add_operator(OperatorMultiplyS,		TypeVoid,		TypeVector,		TypeFloat);
	add_operator(OperatorDivideS,		TypeVoid,		TypeVector,		TypeFloat);
	add_operator(OperatorSubtract,		TypeVector,		TypeVoid,		TypeVector);
	
	msg_db_l(3);
}

void SIAddSuperArrays()
{
	msg_db_r("SIAddSuperArrays", 3);

	for (int i=0;i<PreType.num;i++)
		if (PreType[i]->IsSuperArray){
			//msg_error(string("super array:  ", PreType[i]->Name));
			script_make_super_array(PreType[i]);
		}
	
	msg_db_l(3);
}

void SIAddCommands()
{
	msg_db_r("SIAddCommands", 3);
	
	// type casting
	add_func("s2i",				TypeInt,		(void*)&s2i);
		func_add_param("s",		TypeString);
	add_func("s2f",				TypeFloat,		(void*)&s2f);
		func_add_param("s",		TypeString);
	add_func("i2s",				TypeString,	(void*)&i2s);
		func_add_param("i",		TypeInt);
	add_func("f2s",				TypeString,		(void*)&f2s);
		func_add_param("f",			TypeFloat);
		func_add_param("decimals",	TypeInt);
	add_func("f2sf",			TypeString,		(void*)&f2sf);
		func_add_param("f",			TypeFloat);
	add_func("b2s",				TypeString,	(void*)&b2s);
		func_add_param("b",		TypeBool);
	add_func("p2s",				TypeString,	(void*)&p2s);
		func_add_param("p",		TypePointer);
	add_func("v2s",				TypeString,	(void*)&fff2s);
		func_add_param("v",		TypeVector);
	add_func("complex2s",		TypeString,	(void*)&ff2s);
		func_add_param("z",		TypeComplex);
	add_func("quaternion2s",	TypeString,	(void*)&ffff2s);
		func_add_param("q",		TypeQuaternion);
	add_func("plane2s",			TypeString,	(void*)&ffff2s);
		func_add_param("p",		TypePlane);
	add_func("color2s",			TypeString,	(void*)&ffff2s);
		func_add_param("c",		TypeColor);
	add_func("rect2s",			TypeString,	(void*)&ffff2s);
		func_add_param("r",		TypeRect);
	add_func("ia2s",			TypeString,	(void*)&ia2s);
		func_add_param("a",		TypeIntList);
	add_func("fa2s",			TypeString,	(void*)&fa2s);
		func_add_param("a",		TypeFloatList);
	add_func("ba2s",			TypeString,	(void*)&ba2s);
		func_add_param("a",		TypeBoolList);
	add_func("sa2s",			TypeString,	(void*)&sa2s);
		func_add_param("a",		TypeStringList);
	add_func("hash_func",		TypeInt, (void*)&hash_func);
		func_add_param("s",		TypeString);
	// debug output
	/*add_func("cprint",			TypeVoid,		(void*)&_cstringout);
		func_add_param("str",	TypeCString);*/
	add_func("print",			TypeVoid,		(void*)&_stringout);
		func_add_param("str",	TypeString);
	// memory
	add_func("_malloc_",			TypePointer,		(void*)&malloc);
		func_add_param("size",	TypeInt);
	add_func("_free_",			TypeVoid,		(void*)&free);
		func_add_param("p",	TypePointer);
	// system
	add_func("_exec_",			TypeString,		(void*)&shell_execute);
		func_add_param("cmd",	TypeString);


// add_func("ExecuteScript",	TypeVoid);
//		func_add_param("filename",		TypeString);
	
	msg_db_l(3);
}

void SIAddPackageFile();
void SIAddPackageMath();
void SIAddPackageThread();
void SIAddPackageHui();
void SIAddPackageNix();
void SIAddPackageNet();
void SIAddPackageImage();
void SIAddPackageSound();
void SIAddPackageX();

void ScriptInit()
{
	msg_db_r("ScriptInit", 1);

	AsmInit();

	SIAddPackageBase();
	SIAddBasicCommands();




	SIAddPackageFile();
	SIAddPackageMath();
	SIAddPackageImage();
	SIAddPackageHui();
	SIAddPackageNix();
	SIAddPackageNet();
	SIAddPackageSound();
	SIAddPackageThread();
	SIAddPackageX();

	cur_package_index = 0;
	cur_package = &Package[0];
	SIAddCommands();
	
	SIAddOperators();
	SIAddSuperArrays();





	add_type_cast(10,	TypeInt,		TypeFloat,	"i2f",	(void*)&CastInt2Float);
	add_type_cast(20,	TypeFloat,		TypeInt,	"f2i",	(void*)&CastFloat2Int);
	add_type_cast(10,	TypeInt,		TypeChar,	"i2c",	(void*)&CastInt2Char);
	add_type_cast(20,	TypeChar,		TypeInt,	"c2i",	(void*)&CastChar2Int);
	add_type_cast(50,	TypePointer,	TypeBool,	"p2b",	(void*)&CastPointer2Bool);
	add_type_cast(50,	TypeInt,		TypeString,	"i2s",	(void*)&CastInt2StringP);
	add_type_cast(50,	TypeFloat,		TypeString,	"f2sf",	(void*)&CastFloat2StringP);
	add_type_cast(50,	TypeBool,		TypeString,	"b2s",	(void*)&CastBool2StringP);
	add_type_cast(50,	TypePointer,	TypeString,	"p2s",	(void*)&CastPointer2StringP);
	add_type_cast(50,	TypeVector,		TypeString,	"v2s",	(void*)&CastVector2StringP);
	add_type_cast(50,	TypeComplex,	TypeString,	"complex2s",	(void*)&CastComplex2StringP);
	add_type_cast(50,	TypeColor,		TypeString,	"color2s",	(void*)&CastFFFF2StringP);
	add_type_cast(50,	TypeQuaternion,	TypeString,	"quaternion2s",	(void*)&CastFFFF2StringP);
	add_type_cast(50,	TypePlane,		TypeString,	"plane2s",	(void*)&CastFFFF2StringP);
	add_type_cast(50,	TypeRect,		TypeString,	"rect2s",	(void*)&CastFFFF2StringP);
	//add_type_cast(50,	TypeClass,		TypeString,	"f2s",	(void*)&CastFloat2StringP);
	add_type_cast(50,	TypeIntList,	TypeString,	"ia2s",	NULL);
	add_type_cast(50,	TypeFloatList,	TypeString,	"fa2s",	NULL);
	add_type_cast(50,	TypeBoolList,	TypeString,	"ba2s",	NULL);
	add_type_cast(50,	TypeStringList,	TypeString,	"sa2s",	NULL);

	/*msg_write("------------------test");
	foreach(PreType, t){
		if (t->SubType)
			msg_write(t->SubType->Name);
	}
	foreach(PreCommand, c){
		msg_write("-----");
		msg_write(c.Name);
		msg_write(c.ReturnType->Name);
		foreach(c.Param, p)
			msg_write(p.Type->Name);
	}
	foreach(PreExternalVar, v){
		msg_write(v.Name);
		msg_write(v.Type->Name);
	}*/


	msg_db_l(1);
}

void ScriptResetSemiExternalData()
{
	msg_db_r("ScriptResetSemiExternalData", 2);
	for (int i=PreExternalVar.num-1;i>=0;i--)
		if (PreExternalVar[i].IsSemiExternal)
			PreExternalVar.erase(i);
	for (int i=PreCommand.num-1;i>=0;i--)
		if (PreCommand[i].IsSemiExternal)
			PreCommand.erase(i);
	msg_db_l(2);
}

// program variables - specific to the surrounding program, can't always be there...
void ScriptLinkSemiExternalVar(const string &name, void *pointer)
{
	msg_db_r("ScriptLinkSemiExternalVar", 2);
	sPreExternalVar v;
	v.Name = name;
	v.Pointer = pointer;
	v.Type = TypeUnknown; // unusable until defined via "extern" in the script!
	v.IsSemiExternal = true; // ???
	PreExternalVar.add(v);
	msg_db_l(2);
}

// program functions - specific to the surrounding program, can't always be there...
void ScriptLinkSemiExternalFunc(const string &name, void *pointer)
{
	sPreCommand c;
	c.Name = name;
	c.IsClassFunction = false;
	c.Func = pointer;
	c.ReturnType = TypeUnknown; // unusable until defined via "extern" in the script!
	c.IsSemiExternal = true;
	PreCommand.add(c);
}

void ScriptEnd()
{
	msg_db_r("ScriptEnd", 1);
	DeleteAllScripts(true, true);

	ScriptResetSemiExternalData();

	PreOperator.clear();

	for (int i=0;i<PreType.num;i++)
		delete(PreType[i]);
	PreType.clear();

	PreConstant.clear();
	PreExternalVar.clear();
	msg_db_l(1);
}

