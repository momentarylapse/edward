
void set_cur_package(const string &name);
sType *add_type(const string &name, int size);
sType *add_type_p(const string &name, sType *sub_type, bool is_silent = false);
sType *add_type_a(const string &name, sType *sub_type, int array_length);
int add_func(const string &name, sType *return_type, void *func, bool is_class = false);
int add_func_special(const string &name, sType *return_type, int index);
void func_add_param(const string &name, sType *type);
void add_class(sType *root_type);
void class_add_element(const string &name, sType *type, int offset);
void class_add_func(const string &name, sType *return_type, void *func);
void add_const(const string &name, sType *type, void *value);
void add_ext_var(const string &name, sType *type, void *var);

typedef void (CFile::*tmf)();
typedef char *tcpa[4];
static void *mf(tmf vmf)
{
	tcpa *cpa=(tcpa*)&vmf;
	return (*cpa)[0];
}
