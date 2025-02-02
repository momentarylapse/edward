#include "language.h"
#include "Resource.h"
//#include "internal.h"
#include "../os/msg.h"



namespace xhui
{

// language
bool _using_language_;
Array<Language> _languages_;
Language *cur_lang = nullptr;


bool Language::Command::match(const string &_ns, const string &_id) {
	if (this->id != _id)
		return false;
	return ((this->_namespace == "") or (this->_namespace == _ns));
}


void set_language(const string &language) {
	cur_lang = nullptr;
	_using_language_ = false;
	for (Language &l: _languages_)
		if (l.name == language) {
			cur_lang = &l;
			_using_language_ = true;
		}
	if (!_using_language_)
		msg_error("HuiSetLanguage: language not found: " + language);

	UpdateAll();
}

// first try the specific namespace, then the global one
#define FOR_LANG_RET(NS, ID, CMD) \
for (auto &c: cur_lang->cmd) \
	if (c.match(NS, ID) and c._namespace != "") \
		return CMD; \
for (auto &c: cur_lang->cmd) \
	if (c.match(NS, ID)) \
		return CMD;

string get_language(const string &ns, const string &id) {
	if ((!_using_language_) or (id.num == 0))
		return "";
	for (auto &c: cur_lang->cmd)
		if (c.match(ns, id))
			return c.text;
	/*if (cur_lang->cmd[id].num == 0)
		return "???";*/
	return "";
}

string get_language_r(const string &ns, const Resource &cmd) {
	string pre;
	if (cmd.options.num > 0)
		pre = "!" + implode(cmd.options, ",") + "\\";

	if ((!_using_language_) or (cmd.id.num == 0))
		return pre + cmd.title;

	FOR_LANG_RET(ns, cmd.id, pre + c.text);

	if (cmd.options.num > 0) {
		if (cmd.title.head(1) == "!")
			return "!" + implode(cmd.options, ",") + "," + cmd.title.sub(1);
		else
			return pre + cmd.title;
	}
	return pre + cmd.title;
}

// tooltip
string get_language_t(const string &ns, const string &id, const string &tooltip) {
	if (tooltip.num > 0)
		return tooltip;
	if ((!_using_language_) or (id.num == 0))
		return "";
	FOR_LANG_RET(ns, id, c.tooltip);
	return "";
}

// pre-translated...translations
string get_language_s(const string &str) {
	if (!_using_language_)
		return str;
	if (str.head(4) == ":##:")
		return get_language_s(str.sub_ref(4));
	for (Language::Translation &t: cur_lang->trans){
		if (str == t.orig)
			return t.trans;
	}
	return str;
}


string get_lang(const string &ns, const string &id, const string &text, bool allow_keys) {
	if (text.num > 0)
		return text;
	if ((!_using_language_) or (id.num == 0))
		return text;
	FOR_LANG_RET(ns, id, c.text);
	return text;
}


void UpdateAll() {
/*	// update windows
	for (int i=0;i<HuiWindow.num;i++){
		for (int j=0;j<HuiWindow[i]->Control.num;j++){
			string id = HuiWindow[i]->Control[j]->ID;
			if (cur_lang->Text[id].num > 0)
				HuiWindow[i]->SetString(id,HuiGetLanguage(id));
		}

		// update menu
		if (HuiWindow[i]->Menu)
			UpdateMenuLanguage(HuiWindow[i]->Menu);
	}*/
}

Array<string> get_languages() {
	Array<string> n;
	for (Language &l: _languages_)
		n.add(l.name);
	return n;
}

string get_cur_language() {
	if (cur_lang)
		return cur_lang->name;
	return "";
}

};
