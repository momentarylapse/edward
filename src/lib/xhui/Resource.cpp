#include "xhui.h"
#include "Resource.h"
#include "Dialog.h"
#include "language.h"
#include "Menu.h"
#include "../os/file.h"
#include "../os/filesystem.h"
#include "../os/formatter.h"
#include "../os/msg.h"

namespace xhui
{

extern Array<Language> _languages_;
Array<Resource> _resources_;

Array<layout::Option> parse_options(const string& s) {
	Array<layout::Option> r;
	for (const auto& x: s.explode(","))
		r.add(layout::Option::parse(x));
	return r;
}

void load_resource_command7(Stream *f, Resource *c) {
	c->type = f->read_str();
	c->id = f->read_str();
	if (c->id == "?")
		c->id = "id:" + i2s(randi(10000000));
	c->options = parse_options(f->read_str());
	c->x = f->read_int();
	c->y = f->read_int();
	int n = f->read_int();
	for (int i=0; i<n; i++) {
		Resource child;
		load_resource_command7(f, &child);
		c->children.add(child);
	}
}

void resource_post_process(Resource &res) {
	if ((res.type == "Dialog") or (res.type == "Window")) {
		if (res.has("headerbar")) {
			if (res.children.num >= 0 and res.children[0].type == "Grid")
				if (res.children[0].children.num >= 0) {
					auto &c = res.children[0].children.back();
					if (c.type == "Grid" and c.has("buttonbar")) {
						Resource hb;
						hb.id = ":header:";
						hb.type = "HeaderBar";
						hb.children = c.children;
						res.children.add(hb);
						res.children[0].children.pop();
					}
				}
		}
	}
}

void load_resource(const Path &filename) {
	// dirty...
	_resources_.clear();
	_languages_.clear();

	try{
		auto f = os::fs::open(filename, "rt");
		int ffv = f->read_str().sub_ref(1)._int();
		if (ffv != 7) {
			delete f;
			msg_error("hui resource version is " + i2s(ffv) + " (7 expected)");
			return;
		}

		f->read_comment();
		int nres = f->read_int();
		for (int i=0;i<nres;i++) {
			Resource res;
			res.children.clear();
			f->read_comment();
			load_resource_command7(f, &res);
			resource_post_process(res);
			_resources_.add(res);
		}

		// languages
		f->read_comment();
		int nl = f->read_int();
		for (int l=0;l<nl;l++) {
			Language hl;

			// Language
			f->read_comment();
			hl.name = f->read_str();

			//  NumIDs
			f->read_comment();
			int n = f->read_int();
			f->read_comment(); // Text
			for (int i=0;i<n;i++) {
				Language::Command c;
				Array<string> ids = f->read_str().explode("/");
				if (ids.num >= 2)
					c._namespace = ids[0];
				if (ids.num >= 1)
					c.id = ids.back();
				c.text = f->read_str().unescape();
				c.tooltip = f->read_str().unescape();
				hl.cmd.add(c);
			}
			// Num Language Strings
			f->read_comment();
			n = f->read_int();
			// Text
			f->read_comment();
			for (int i=0;i<n;i++) {
				Language::Translation s;
				s.orig = f->read_str().unescape();
				s.trans = f->read_str().unescape();
				hl.trans.add(s);
			}
			_languages_.add(hl);
		}
		delete f;
	} catch (Exception &e) {
		msg_error(e.message());
	}
}

Resource *get_resource(const string &id) {
	for (Resource &r: _resources_)
		if (r.id == id)
			return &r;
	if (id.num > 0)
		msg_error("hui resource not found: " + id);
	return nullptr;
}

Dialog *create_resource_dialog(const string &id, Window *root) {
	//return HuiCreateDialog("-dialog not found in resource-",200,100,root,true,mf);
	Resource *res = get_resource(id);
	if (!res) {
		msg_error(format("CreateResourceDialog  (id=%s)  m(-_-)m", id));
		return nullptr;
	}
	

	if ((res->type != "Dialog") and (res->type != "Window")) {
		msg_error("resource type should be Dialog or Window, but is " + res->type);
		return nullptr;
	}

	string menu_id = res->value("menu");
	string toolbar_id = res->value("toolbar");
	bool allow_parent = res->has("allow-root") or res->has("allow-parent");

	// dialog
	int width = res->value("width", "300")._int();
	int height = res->value("height", "250")._int();
	Dialog* dlg = new Dialog(get_language_r(res->id, *res), width, height, root);//, allow_parent);

#if 0
	// menu?
	if (menu_id.num > 0)
		dlg->set_menu(create_resource_menu(menu_id, dlg));

	// toolbar?
	if (toolbar_id.num > 0)
		if (auto t = dlg->get_toolbar(TOOLBAR_TOP))
			t->set_by_id(toolbar_id);
#endif

	// controls
	for (Resource &cmd: res->children)
		dlg->_add_control(id, cmd, "");

	return dlg;
	
	/*msg_error(format("HuiCreateResourceDialog  (id=%d)  m(-_-)m",id));
	CHuiWindow *d=HuiCreateDialog(format("-dialog (id=%d) not found in resource-",id),300,200,root,true,mf);
	return d;*/
}

#if 0
xfer<Menu> _create_res_menu_(const string &ns, Resource *res, Panel *panel) {
	Menu *menu = new Menu(panel);

	for (Resource &c: res->children) {
		if (c.type == "Item") {
			if (sa_contains(c.options, "checkable"))
				menu->add_checkable(get_lang(ns, c.id, c.title, true), c.id);
			else if (c.image().num > 0)
				menu->add_with_image(get_lang(ns, c.id, c.title, true), c.image(), c.id);
			else
				menu->add(get_lang(ns, c.id, c.title, true), c.id);
		} else if (c.type == "Separator") {
			menu->add_separator();
		} else if (c.type == "Menu") {
			Menu *sub = _create_res_menu_(ns, &c, panel);
			menu->add_sub_menu(get_lang(ns, c.id, c.title, true), c.id, sub);
		}

		if (sa_contains(c.options, "disabled"))
			menu->items.back()->enable(false);
	}
	return menu;
}

xfer<Menu> create_resource_menu(const string &id, Panel *panel) {
	Resource *res = get_resource(id);
	if (!res) {
		msg_error(format("CreateResourceMenu  (id=%s)  m(-_-)m", id));
		throw Exception(format("CreateResourceMenu  (id=%s)  m(-_-)m", id));
		return nullptr;
	}

	return _create_res_menu_(id, res, panel);
}

xfer<Menu> create_menu_from_source(const string &source, Panel *panel) {
	Resource res = parse_resource(source);

	return _create_res_menu_(res.id, &res, panel);
}
#endif


Resource parse_resource(const string &buffer, bool literally) {
	return layout::parse_resource(buffer, literally);
}

xfer<Menu> create_resource_menu(const string& ns, const Resource* r) {
	auto m = new Menu;
	for (const auto& rr: r->children) {
		if (rr.type == "Separator")
			continue;
		if (rr.type == "Menu")
			m->add_item_menu(rr.id, get_language_r(ns, rr), create_resource_menu(ns, &rr));
		else
			m->add_item(rr.id, get_language_r(ns, rr));
		m->items.back().enabled = rr.enabled();
	}
	return m;
}

xfer<Menu> create_resource_menu(const string &id) {
	if (auto r = get_resource(id))
		return create_resource_menu(id, r);
	return nullptr;
}

};

