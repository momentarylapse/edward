
// character buffer and expressions (syntax analysis)

struct ps_exp_t
{
	char* name; // points into Exp.buffer
	int pos;
};

struct ps_line_t
{
	int physical_line, length, indent;
	Array<ps_exp_t> exp;
};

struct ps_exp_buffer_t
{
	char *buffer; // holds ALL expressions of the file (0 separated)
	char *buf_cur; // pointer to the latest one
	Array<ps_line_t> line;
	ps_line_t temp_line;
	ps_line_t *cur_line;
	int cur_exp;
	int comment_level;
	string _cur_;
};


#define cur_name		Exp._cur_
#define get_name(n)		string(Exp.cur_line->exp[n].name)
#define next_exp()		{Exp.cur_exp ++; Exp._cur_ = Exp.cur_line->exp[Exp.cur_exp].name;}//;ExpectNoNewline()
#define rewind_exp()	{Exp.cur_exp --; Exp._cur_ = Exp.cur_line->exp[Exp.cur_exp].name;}
#define end_of_line()	(Exp.cur_exp >= Exp.cur_line->exp.num - 1) // the last entry is "-eol-"
#define past_end_of_line()	(Exp.cur_exp >= Exp.cur_line->exp.num)
#define next_line()		{Exp.cur_line ++; Exp.cur_exp = 0; test_indent(Exp.cur_line->indent);  Exp._cur_ = Exp.cur_line->exp[Exp.cur_exp].name;}
#define end_of_file()	((long)Exp.cur_line >= (long)&Exp.line[Exp.line.num - 1]) // last line = "-eol-"


// macros
struct sDefine
{
	string Source;
	Array<string> Dest;
};

// single enum entries
/*struct sEnum
{
	string Name;
	int Value;
};*/

// for any type of constant used in the script
struct sConstant
{
	CPreScript *owner;
	string name;
	char *data;
	sType *type;
};

enum
{
	KindUnknown,
	// data
	KindVarLocal,
	KindVarGlobal,
	KindVarFunction,
	KindVarExternal,		// = variable from surrounding program
	KindEnum,				// = single enum entry
	KindConstant,
	// execution
	KindFunction,			// = script defined functions
	KindCompilerFunction,	// = compiler functions
	KindBlock,				// = block of commands {...}
	KindOperator,
	KindPrimitiveOperator,	// provisorical...
	// data altering
	KindAddressShift,		// = . "struct"
	KindArray,				// = []
	KindPointerAsArray,		// = []
	KindReference,			// = &
	KindDereference,		// = *
	KindDerefAddressShift,	// = ->
	KindRefToLocal,
	KindRefToGlobal,
	KindRefToConst,
	KindAddress,			// &global (for pre processing address shifts)
	KindMemory,				// global (but LinkNr = address)
	KindLocalAddress,		// &local (for pre processing address shifts)
	KindLocalMemory,		// local (but LinkNr = address)
	// special
	KindType,
	// compilation
	KindVarTemp,
	KindDerefVarTemp,
	KindDerefVarLocal,
	KindRegister,
	KindDerefRegister,
	KindMarker,
	KindAsmBlock,
};

// type of expression (syntax)
enum
{
	ExpKindNumber,
	ExpKindLetter,
	ExpKindSpacing,
	ExpKindSign
};

struct sCommand;

// {...}-block
struct sBlock
{
	int Root;
	int Index;
	Array<sCommand*> Command; // ID of command in global command array
};

struct sLocalVariable
{
	sType *Type; // for creating instances
	string Name;
	int _Offset; // for compilation
};

// user defined functions
struct sFunction
{
	string Name;
	// parameters (linked to intern variables)
	int NumParams;
	// block of code
	sBlock *Block;
	// local variables
	Array<sLocalVariable> Var;
	sType *LiteralParamType[SCRIPT_MAX_PARAMS];
	sType *Class;
	// return value
	sType *Type;
	sType *LiteralType;
	// for compilation...
	int _VarSize, _ParamSize;
};

// single operand/command
struct sCommand
{
	int Kind, LinkNr;
	CScript *script;
	// parameters
	int NumParams;
	sCommand *Param[SCRIPT_MAX_PARAMS];
	// linking of class function instances
	sCommand *Instance;
	// return value
	sType *Type;
};

struct sAsmBlock
{
	char *block;
	int Line;
};

class CScript;


// data structures (uncompiled)
class CPreScript
{
public:
	CPreScript(CScript *_script);
	~CPreScript();

	void LoadAndParseFile(const string &filename, bool just_analyse);
	bool LoadToBuffer(const string &filename, bool just_analyse);
	void AddIncludeData(CScript *s);

	bool Error, IncludeLinkerError;
	string ErrorMsg, ErrorMsgExt[2];
	int ErrorLine,ErrorColumn;
	void DoError(const string &msg, int overwrite_line = -1);
	bool ExpectNoNewline();
	bool ExpectNewline();
	bool ExpectIndent();

	// lexical analysis
	int GetKind(char c);
	void Analyse(const char *buffer, bool just_analyse);
	bool AnalyseExpression(const char *buffer, int &pos, ps_line_t *l, int &line_no, bool just_analyse);
	bool AnalyseLine(const char *buffer, ps_line_t *l, int &line_no, bool just_analyse);
	void AnalyseLogicalLine(const char *buffer, ps_line_t *l, int &line_no, bool just_analyse);
	
	// syntax analysis
	void Parser();
	void ParseEnum();
	void ParseClass();
	void ParseFunction(sType *class_type, bool as_extern);
	void ParseClassFunction(sType *t, bool as_extern);
	sType *ParseVariableDefSingle(sType *type, sFunction *f, bool as_param = false);
	void ParseVariableDef(bool single, sFunction *f);
	void ParseGlobalConst(const string &name, sType *type);
	int WhichPrimitiveOperator(const string &name);
	int WhichCompilerFunction(const string &name);
	void CommandSetCompilerFunction(int CF,sCommand *Com);
	int WhichExternalVariable(const string &name);
	int WhichType(const string &name);
	void SetExternalVariable(int gv, sCommand *c);
	void AddType();

	// pre compiler
	void PreCompiler(bool just_analyse);
	void HandleMacro(ps_line_t *l, int &line_no, int &NumIfDefs, bool *IfDefed, bool just_analyse);
	void CreateImplicitFunctions(sType *t, bool relocate_last_function);
	void CreateAllImplicitFunctions(bool relocate_last_function);

	// syntax analysis
	sType *GetConstantType();
	void *GetConstantValue();
	sType *GetType(const string &name, bool force);
	void AddType(sType **type);
	sType *CreateNewType(const string &name, int size, bool is_pointer, bool is_silent, bool is_array, int array_size, sType *sub);
	void TestArrayDefinition(sType **type, bool is_pointer);
	bool GetExistence(const string &name, sFunction *f);
	void LinkMostImportantOperator(int &NumOperators, sCommand **Operand, sCommand **Operator, int *op_exp);
	bool LinkOperator(int op_no, sCommand *param1, sCommand *param2, sCommand **cmd);
	void GetOperandExtension(sCommand *Operand, sFunction *f);
	sCommand *GetCommand(sFunction *f);
	void GetCompleteCommand(sBlock *block, sFunction *f);
	sCommand *GetOperand(sFunction *f);
	sCommand *GetOperator(sFunction *f);
	void FindFunctionParameters(int &np, sType **WantedType, sFunction *f, sCommand *cmd);
	void FindFunctionSingleParameter(int p, sType **WantedType, sFunction *f, sCommand *cmd);
	void GetFunctionCall(const string &f_name, sCommand *Operand, sFunction *f);
	bool GetSpecialFunctionCall(const string &f_name, sCommand *Operand, sFunction *f);
	void CheckParamLink(sCommand *link, sType *type, const string &f_name = "", int param_no = -1);
	void GetSpecialCommand(sBlock *block, sFunction *f);

	// neccessary conversions
	void ConvertCallByReference();
	void BreakDownComplicatedCommands();
	void BreakDownHighLevelOperators();
	void MapLocalVariablesToStack();

	// data creation
	int AddVar(const string &name, sType *type, sFunction *f);
	int AddConstant(sType *type);
	sBlock *AddBlock();
	sFunction *AddFunction(const string &name, sType *type);
	sCommand *AddCommand();

	// pre processor
	void PreProcessCommand(CScript *s, sCommand *c);
	void PreProcessor(CScript *s);
	void PreProcessCommandAddresses(CScript *s, sCommand *c);
	void PreProcessorAddresses(CScript *s);
	void Simplify();

	// debug displaying
	void ShowCommand(sCommand *c);
	void ShowFunction(int f);
	void ShowBlock(sBlock *b);
	void Show();

// data

	string Filename;
	string Buffer;
	int BufferLength, BufferPos;
	ps_exp_buffer_t Exp;
	sCommand GetExistenceLink;

	// compiler options
	bool FlagShowPrae;
	bool FlagShow;
	bool FlagDisassemble;
	bool FlagNoExecution;
	bool FlagImmortal;
	bool FlagCompileOS;
	bool FlagCompileInitialRealMode;
	bool FlagOverwriteVariablesOffset;
	int VariablesOffset;

	int NumOwnTypes;
	Array<sType*> Type;
	//Array<sEnum> Enum;
	Array<CScript*> Include;
	Array<sDefine> Define;
	char *AsmMetaInfo;
	Array<sAsmBlock> AsmBlock;
	Array<sConstant> Constant;
	Array<sBlock*> Block;
	Array<sFunction*> Function;
	Array<sCommand*> Command;

	sFunction RootOfAllEvil;

	CScript *script;
	sFunction *cur_func;
};

#define _do_error_(str,n,r)	{DoError(str);msg_db_l(n);return r;}
#define _do_error_int_(str,n,r)	{DoErrorInternal(str);msg_db_l(n);return r;}
#define _return_(n,r)		{msg_db_l(n);return r;}

string Kind2Str(int kind);
string Operator2Str(CPreScript *s,int cmd);
void clear_exp_buffer(ps_exp_buffer_t *e);
void CreateAsmMetaInfo(CPreScript* ps);
extern CScript *cur_script;



inline bool isNumber(char c)
{
	if ((c>=48)&&(c<=57))
		return true;
	return false;
}

inline bool isLetter(char c)
{
	if ((c>='a')&&(c<='z'))
		return true;
	if ((c>='A')&&(c<='Z'))
		return true;
	if ((c=='_'))
		return true;
	// Umlaute
#ifdef HUI_OS_WINDOWS
	// Windows-Zeichensatz
	if ((c==-28)||(c==-10)||(c==-4)||(c==-33)||(c==-60)||(c==-42)||(c==-36))
		return true;
#endif
#ifdef HUI_OS_LINUX
	// Linux-Zeichensatz??? testen!!!!
#endif
	return false;
}

inline bool isSpacing(char c)
{
	if ((c==' ')||(c=='\t')||(c=='\n'))
		return true;
	return false;
}

inline bool isSign(char c)
{
	if ((c=='.')||(c==':')||(c==',')||(c==';')||(c=='+')||(c=='-')||(c=='*')||(c=='%')||(c=='/')||(c=='=')||(c=='<')||(c=='>')||(c=='\''))
		return true;
	if ((c=='(')||(c==')')||(c=='{')||(c=='}')||(c=='&')||(c=='|')||(c=='!')||(c=='[')||(c==']')||(c=='\"')||(c=='\\')||(c=='#')||(c=='?')||(c=='$'))
		return true;
	return false;
}

