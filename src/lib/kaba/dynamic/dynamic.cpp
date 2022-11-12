#include "dynamic.h"
#include "../kaba.h"
#include "exception.h"
#include "call.h"
#include "../../any/any.h"
#include "../../base/callable.h"
#include "../../base/sort.h"
#include "../../os/msg.h"
#include <algorithm>

namespace kaba {
	
extern const Class *TypeIntList;
extern const Class *TypeFloatList;
extern const Class *TypeBoolList;
extern const Class *TypeAny;
extern const Class *TypePath;


	
	

#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")


void array_inplace_reverse(DynamicArray &array) {
	if (array.element_size == 4)
		inplace_reverse(*reinterpret_cast<Array<int>*>(&array));
	else if (array.element_size == 1)
		inplace_reverse(*reinterpret_cast<Array<char>*>(&array));
	else if (array.element_size == 8)
		inplace_reverse(*reinterpret_cast<Array<int64>*>(&array));
	else
		array.reverse();
}

template<class F>
void array_inplace_quick_sort(DynamicArray &array, int first, int last, F f) {
	if (first < 0 or last < 0 or first >= last)
		return;
	auto partition = [first, last] (DynamicArray &array, F f) {
		int ipivot = (first + last) / 2;
		int left = first-1;
		int right = last+1;
		while (true) {
			void *pivot = array.simple_element(ipivot);
			if (left != ipivot)
				left ++;
			if (right != ipivot)
				right --;
			while (!f(pivot, array.simple_element(left))) // A < P  <=>  !(P <= A)
				left ++;
			while (!f(array.simple_element(right), pivot)) // A > P  <=>  !(A <= P)
				right --;
			if (left >= right)
				return right;
			array.simple_swap(left, right);
			if (left == ipivot)
				ipivot = right;
			else if (right == ipivot)
				ipivot = left;
		}
		return 0;
	};
	int p = partition(array, f);
	array_inplace_quick_sort(array, first, p-1, f);
	array_inplace_quick_sort(array, p+1, last, f);
}

template<class T>
void _array_sort(DynamicArray &array, int offset_by) {
	auto f = [offset_by] (const void *a, const void *b) {
		return *(T*)((const char*)a + offset_by) <= *(T*)((const char*)b + offset_by);
	};
	array_inplace_quick_sort(array, 0, array.num-1, f);
}

template<class T>
void _array_sort_p(DynamicArray &array, int offset_by) {
	auto f = [offset_by] (const void *a, const void *b) {
		auto *aa = (const T*) ((const char*)a + offset_by);
		auto *bb = (const T*) ((const char*)b + offset_by);
		return (*aa <= *bb);
	};
	inplace_sort(*(Array<void*>*)&array, f);
}

template<class T>
void _array_sort_pf(DynamicArray &array, Function *func) {
	auto f = [func] (void **a, void **b) {
		T r1, r2;
		if (!call_function(func, &r1, {*a}) or !call_function(func, &r2, {*b}))
			kaba_raise_exception(new KabaException("call failed " + func->long_name()));
		return (r1 <= r2);
	};
	inplace_sort(*(Array<void**>*)&array, f);
}

void var_assign(void *pa, const void *pb, const Class *type) {
	if ((type == TypeInt) or (type == TypeFloat32)) {
		*(int*)pa = *(int*)pb;
	} else if ((type == TypeBool) or (type == TypeChar)) {
		*(char*)pa = *(char*)pb;
	} else if (type->is_pointer()) {
		*(void**)pa = *(void**)pb;
	} else {
		auto *f = type->get_assign();
		if (!f)
			kaba_raise_exception(new KabaException("can not assign variables of type " + type->long_name()));
		typedef void func_t(void*, const void*);
		auto *ff = (func_t*)f->address;
		ff(pa, pb);
	}
}

void var_init(void *p, const Class *type) {
	//msg_write("init " + type->long_name());
	if (!type->needs_constructor())
		return;
	auto *f = type->get_default_constructor();
	if (!f)
		kaba_raise_exception(new KabaException("can not init a variable of type " + type->long_name()));
	typedef void func_t(void*);
	auto *ff = (func_t*)f->address;
	ff(p);
}

void array_clear(void *p, const Class *type) {
	auto *f = type->get_member_func("clear", TypeVoid, {});
	if (!f)
		kaba_raise_exception(new KabaException("can not clear an array of type " + type->long_name()));
	typedef void func_t(void*);
	auto *ff = (func_t*)f->address;
	ff(p);
}

void array_resize(void *p, const Class *type, int num) {
	auto *f = type->get_member_func("resize", TypeVoid, {TypeInt});
	if (!f)
		kaba_raise_exception(new KabaException("can not resize an array of type " + type->long_name()));
	typedef void func_t(void*, int);
	auto *ff = (func_t*)f->address;
	ff(p, num);
}

void array_add(DynamicArray &array, void *p, const Class *type) {
	//msg_write("array add " + type->long_name());
	if ((type == TypeIntList) or (type == TypeFloatList)) {
		array.append_4_single(*(int*)p);
	} else if (type == TypeBoolList) {
		array.append_1_single(*(char*)p);
	} else {
		auto *f = type->get_member_func("add", TypeVoid, {type->param[0]});
		if (!f)
			kaba_raise_exception(new KabaException("can not add to array type " + type->long_name()));
		typedef void func_t(void*, const void*);
		auto *ff = (func_t*)f->address;
		ff(&array, p);
	}
}

DynamicArray _cdecl array_sort(DynamicArray &array, const Class *type, const string &_by) {
	if (!type->is_super_array())
		kaba_raise_exception(new KabaException(format("type '%s' is not an array", type->name)));
	const Class *el = type->param[0];
	if (array.element_size != el->size)
		kaba_raise_exception(new KabaException(format("element type size mismatch... type=%s: %d  vs  array: %d", el->name, el->size, array.element_size)));

	DynamicArray rr;
	var_init(&rr, type);
	var_assign(&rr, &array, type);

	const Class *rel = el;

	if (el->is_pointer())
		rel = el->param[0];

	string by = _by;
	bool reverse = false;
	if (_by.head(1) == "-") {
		by = by.sub(1);
		reverse = true;
	}

	int offset = -1;
	const Class *by_type = nullptr;
	Function *sfunc = nullptr;
	if (by == "") {
		offset = 0;
		by_type = rel;
	} else {
		for (auto &e: rel->elements)
			if (e.name == by) {
				by_type = e.type;
				offset = e.offset;
			}
		if (!by_type) {
			for (auto *f: weak(rel->functions))
				if (f->name == by) {
					if (f->num_params != 1)
						kaba_raise_exception(new KabaException("can only sort by a member function without parameters"));
					by_type = f->literal_return_type;
					sfunc = f;
				}
			if (!sfunc)
				kaba_raise_exception(new KabaException("type '" + rel->name + "' does not have an element '" + by + "'"));
		}
	}

	if (sfunc) {
		if (!el->is_pointer())
			kaba_raise_exception(new KabaException("function sorting only for pointers"));
		if (by_type == TypeString)
			_array_sort_pf<string>(rr, sfunc);
		else if (by_type == TypePath)
			_array_sort_pf<Path>(rr, sfunc);
		else if (by_type == TypeInt)
			_array_sort_pf<int>(rr, sfunc);
		else if (by_type == TypeFloat32)
			_array_sort_pf<float>(rr, sfunc);
		else if (by_type == TypeBool)
			_array_sort_pf<bool>(rr, sfunc);
		else
			kaba_raise_exception(new KabaException("can't sort by function '" + by_type->long_name() + "' yet"));
	} else if (el->is_pointer()) {
		if (by_type == TypeString)
			_array_sort_p<string>(rr, offset);
		else if (by_type == TypePath)
			_array_sort_p<Path>(rr, offset);
		else if (by_type == TypeInt)
			_array_sort_p<int>(rr, offset);
		else if (by_type == TypeFloat32)
			_array_sort_p<float>(rr, offset);
		else if (by_type == TypeBool)
			_array_sort_p<bool>(rr, offset);
		else
			kaba_raise_exception(new KabaException("can't sort by type '" + by_type->long_name() + "' yet"));
	} else {
		if (by_type == TypeString)
			_array_sort<string>(rr, offset);
		else if (by_type == TypePath)
			_array_sort<Path>(rr, offset);
		else if (by_type == TypeInt)
			_array_sort<int>(rr, offset);
		else if (by_type == TypeFloat32)
			_array_sort<float>(rr, offset);
		else if (by_type == TypeBool)
			_array_sort<bool>(rr, offset);
		else
			kaba_raise_exception(new KabaException("can't sort by type '" + by_type->long_name() + "' yet"));
	}
	if (reverse)
		array_inplace_reverse(rr);
	return rr;
}


struct EnumLabel {
	const Class *type;
	int value;
	string label;
};
Array<EnumLabel> all_enum_labels;
void add_enum_label(const Class *type, int value, const string &label) {
	all_enum_labels.add({type, value, label});
}
void remove_enum_labels(const Class *type) {
	for (int i=all_enum_labels.num-1; i>=0; i--)
		if (all_enum_labels[i].type == type)
			all_enum_labels.erase(i);
}
string find_enum_label(const Class *type, int value) {
	// explicit labels
	for (auto &l: all_enum_labels)
		if (l.type == type and l.value == value)
			return l.label;

	// const names
	for (auto c: type->constants)
		if (c->type == type and c->as_int() == value)
			return c->name;

	// not found
	return i2s(value);
}

int enum_parse(const string &label, const Class *type) {
	// explicit labels
	for (auto &l: all_enum_labels)
		if (l.type == type and l.label == label)
			return l.value;

	// const names
	for (auto c: type->constants)
		if (c->type == type and c->name == label)
			return c->as_int();

	if (str_is_integer(label))
		return s2i(label);

	// not found
	return -1;
}



string class_repr(const Class *c) {
	if (c)
		return c->long_name();
	return "nil";
}

// probably deprecated...?
string func_repr(const Function *f) {
	if (f)
		return "<func " + f->long_name() + ">";
	return "<func -nil->";
}


Array<const Class*> get_callable_param_types(const Class *fp);
const Class *get_callable_return_type(const Class *fp);
string make_callable_signature(const Array<const Class*> &param, const Class *ret);
string callable_signature(const Class *type) {
	auto pp = get_callable_param_types(type);
	auto r = get_callable_return_type(type);
	return make_callable_signature(pp, r);
}

string callable_repr(const void *p, const Class *type) {
	return "<callable " + callable_signature(type) + ">";
}

string _cdecl var_repr(const void *p, const Class *type) {
	if (type == TypeInt) {
		return i2s(*(int*)p);
	} else if (type == TypeFloat32) {
		return f2s(*(float*)p, 6);
	} else if (type == TypeFloat64) {
		return f2s((float)*(double*)p, 6);
	} else if (type == TypeBool) {
		return b2s(*(bool*)p);
	} else if (type == TypeClass) {
		return class_repr((Class*)p);
	} else if (type->is_callable_fp() or type->is_callable_bind()) {
		return callable_repr(p, type);
	} else if (type == TypeFunction or type->type == Class::Type::FUNCTION) {
		// probably not...
		return func_repr((Function*)p);
	} else if (type == TypeAny) {
		return ((Any*)p)->repr();
	} else if (type->is_some_pointer()) {
		auto *pp = *(void**)p;
		// auto deref?
		if (pp and (type->param[0] != TypeVoid))
			return var_repr(pp, type->param[0]);
		return p2s(pp);
	} else if (type == TypeString) {
		return ((string*)p)->repr();
	} else if (type == TypeCString) {
		return string((char*)p).repr();
	} else if (type == TypePath) {
		return ((Path*)p)->str().repr();
	} else if (type->is_enum()) {
		return find_enum_label(type, *(int*)p);
	} else if (type->is_super_array()) {
		string s;
		auto *da = reinterpret_cast<const DynamicArray*>(p);
		for (int i=0; i<da->num; i++) {
			if (i > 0)
				s += ", ";
			s += var_repr(((char*)da->data) + i * da->element_size, type->param[0]);
		}
		return "[" + s + "]";
	} else if (type->is_dict()) {
		string s;
		auto *da = reinterpret_cast<const DynamicArray*>(p);
		for (int i=0; i<da->num; i++) {
			if (i > 0)
				s += ", ";
			s += var_repr(((char*)da->data) + i * da->element_size, TypeString);
			s += ": ";
			s += var_repr(((char*)da->data) + i * da->element_size + sizeof(string), type->param[0]);
		}
		return "{" + s + "}";
	} else if (type->elements.num > 0) {
		string s;
		for (auto &e: type->elements) {
			if (e.hidden())
				continue;
			if (s.num > 0)
				s += ", ";
			s += var_repr(((char*)p) + e.offset, e.type);
		}
		return "(" + s + ")";

	} else if (type->is_array()) {
		string s;
		for (int i=0; i<type->array_length; i++) {
			if (i > 0)
				s += ", ";
			s += var_repr(((char*)p) + i * type->param[0]->size, type->param[0]);
		}
		return "[" + s + "]";
	} else if (type->is_enum()) {
		return find_enum_label(type, *(int*)p);
	}
	return d2h(p, type->size);
}

string _cdecl var2str(const void *p, const Class *type) {
	if (type == TypeString)
		return *(string*)p;
	if (type == TypeCString)
		return string((char*)p);
	if (type == TypePath)
		return ((Path*)p)->str();
	if (type == TypeAny)
		return reinterpret_cast<const Any*>(p)->str();
	return var_repr(p, type);
}

Any _cdecl dynify(const void *var, const Class *type) {
	if (type == TypeInt or type->is_enum())
		return Any(*(int*)var);
	if (type == TypeFloat32)
		return Any(*(float*)var);
	if (type == TypeBool)
		return Any(*(bool*)var);
	if (type == TypeString)
		return Any(*(string*)var);
	if (type->is_some_pointer())
		return Any(*(void**)var);
	if (type == TypeAny)
		return *(Any*)var;
	if (type->is_array()) {
		Any a = Any::EmptyArray;
		auto *t_el = type->get_array_element();
		for (int i=0; i<type->array_length; i++)
			a.add(dynify((char*)var + t_el->size * i, t_el));
		return a;
	}
	if (type->is_super_array()) {
		Any a = Any::EmptyArray;
		auto *ar = reinterpret_cast<const DynamicArray*>(var);
		auto *t_el = type->get_array_element();
		for (int i=0; i<ar->num; i++)
			a.add(dynify((char*)ar->data + ar->element_size * i, t_el));
		return a;
	}
	if (type->is_dict()) {
		Any a = Any::EmptyMap;
		auto *da = reinterpret_cast<const DynamicArray*>(var);
		auto *t_el = type->get_array_element();
		for (int i=0; i<da->num; i++) {
			string key = *(string*)(((char*)da->data) + i * da->element_size);
			a.map_set(key, dynify(((char*)da->data) + i * da->element_size + sizeof(string), type->param[0]));
		}
		return a;
	}
	
	// class
	Any a;
	for (auto &e: type->elements) {
		if (!e.hidden())
			a.map_set(e.name, dynify((char*)var + e.offset, e.type));
	}
	return a;
}

Array<const Class*> func_effective_params(const Function *f);

DynamicArray array_map(void *fff, DynamicArray *a, const Class *ti, const Class *to) {
	//msg_write("map " + ti->long_name() + " -> " + to->long_name());

	DynamicArray r;
	r.init(to->size);
	if (to->needs_constructor()) {
		if (to == TypeString) {
			((Array<string>*)&r)->resize(a->num);
		} else  {
			kaba_raise_exception(new KabaException("map(): output type not allowed: " + to->long_name()));
		}
	} else {
		r.simple_resize(a->num);
	}
	for (int i=0; i<a->num; i++) {
		void *po = r.simple_element(i);
		void *pi = a->simple_element(i);
		bool ok = call_callable(fff, po, {pi}, to, {ti});
		if (!ok)
			kaba_raise_exception(new KabaException(format("map(): failed to dynamically call %s -> %s", ti->long_name(), to->long_name())));
	}
	return r;
}

void assert_num_params(Function *f, int n) {
	auto p = func_effective_params(f);
	if (p.num != n)
		kaba_raise_exception(new KabaException("call(): " + i2s(p.num) + " parameters expected, " + i2s(n) + " given"));
}

void assert_return_type(Function *f, const Class *ret) {
	msg_write("TODO check type");
	if (f->literal_return_type != ret)
		kaba_raise_exception(new KabaException("call(): function returns " + f->literal_return_type->long_name() + ", " + ret->long_name() + " required"));
}

void kaba_call0(Function *func) {
	assert_num_params(func, 0);
	assert_return_type(func, TypeVoid);
	if (!call_function(func, nullptr, {}))
		kaba_raise_exception(new KabaException("call(): failed to dynamically call " + func->signature()));
}

void kaba_call1(Function *func, void *p1) {
	assert_num_params(func, 1);
	assert_return_type(func, TypeVoid);
	if (!call_function(func, nullptr, {p1}))
		kaba_raise_exception(new KabaException("call(): failed to dynamically call " + func->signature()));
}

void kaba_call2(Function *func, void *p1, void *p2) {
	assert_num_params(func, 2);
	assert_return_type(func, TypeVoid);
	if (!call_function(func, nullptr, {p1, p2}))
		kaba_raise_exception(new KabaException("call(): failed to dynamically call " + func->signature()));
}

void kaba_call3(Function *func, void *p1, void *p2, void *p3) {
	assert_num_params(func, 3);
	assert_return_type(func, TypeVoid);
	if (!call_function(func, nullptr, {p1, p2, p3}))
		kaba_raise_exception(new KabaException("call(): failed to dynamically call " + func->signature()));
}

#pragma GCC pop_options

	
	
}
