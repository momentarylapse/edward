/*
 * HuiControlDrawingArea.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlDrawingArea.h"
#include "../hui.h"
#include "../hui_internal.h"

#ifdef HUI_API_GTK

int GtkAreaMouseSet = -1;
int GtkAreaMouseSetX, GtkAreaMouseSetY;

gboolean OnGtkAreaDraw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	((HuiControl*)user_data)->Notify("hui:redraw");
	return false;
}

/*void OnGtkAreaResize(GtkWidget *widget, GtkRequisition *requisition, gpointer user_data)
{	NotifyWindowByWidget((CHuiWindow*)user_data, widget, "hui:resize", false);	}*/

template<class T>
void win_set_input(HuiWindow *win, T *event)
{
	win->input.dx = event->x - win->input.x;
	win->input.dy = event->y - win->input.y;
	//msg_write(format("%.0f\t%0.f\t->\t%0.f\t%0.f\t(%.0f\t%0.f)", win->input.x, win->input.y, event->x, event->y, win->input.dx, win->input.dy));
	win->input.dz = 0;
	win->input.x = event->x;
	win->input.y = event->y;
	int mod = event->state;
	win->input.lb = ((mod & GDK_BUTTON1_MASK) > 0);
	win->input.mb = ((mod & GDK_BUTTON2_MASK) > 0);
	win->input.rb = ((mod & GDK_BUTTON3_MASK) > 0);
}

gboolean OnGtkAreaMouseMove(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	if (GtkAreaMouseSet >= 0){
		if ((event->x != GtkAreaMouseSetX) || (event->y != GtkAreaMouseSetY)){
			GtkAreaMouseSet --;
			//msg_write(format("ignore %.0f\t%0.f", event->x, event->y));
			return false;
		}
		GtkAreaMouseSet = -1;
	}
	HuiControl *c = (HuiControl*)user_data;
	win_set_input(c->win, event);
	c->Notify("hui:mouse-move", false);
	gdk_event_request_motions(event); // to prevent too many signals for slow message processing
	return false;
}

gboolean OnGtkAreaButtonDown(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	HuiControl *c = (HuiControl*)user_data;
	win_set_input(c->win, event);
	string msg = "hui:";
	if (event->button == 1)
		msg += "left";
	else if (event->button == 2)
		msg += "middle";
	else if (event->button == 3)
		msg += "right";
	if (event->type == GDK_2BUTTON_PRESS)
		msg += "-double-click";
	else
		msg += "-button-down";
	gtk_widget_grab_focus(widget);
	c->Notify(msg, false);
	return false;
}

gboolean OnGtkAreaButtonUp(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	HuiControl *c = (HuiControl*)user_data;
	win_set_input(c->win, event);
	string msg = "hui:";
	if (event->button == 1)
		msg += "left";
	else if (event->button == 2)
		msg += "middle";
	else if (event->button == 3)
		msg += "right";
	msg += "-button-up";
	c->Notify(msg, false);
	return false;
}

gboolean OnGtkAreaMouseWheel(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
	HuiControl *c = (HuiControl*)user_data;
	if (c->win){
		if (event->direction == GDK_SCROLL_UP)
			c->win->input.dz = 1;
		else if (event->direction == GDK_SCROLL_DOWN)
			c->win->input.dz = -1;
		c->Notify("hui:mouse-wheel", false);
	}
	return false;
}



void _get_hui_key_id_(GdkEventKey *event, int &key, int &key_code)
{
	// convert hardware keycode into GDK keyvalue
	GdkKeymapKey kmk;
	kmk.keycode = event->hardware_keycode;
	kmk.group = event->group;
	kmk.level = 0;
	int keyvalue = gdk_keymap_lookup_key(gdk_keymap_get_default(), &kmk);
	// TODO GTK3
	//int keyvalue = event->keyval;
	//msg_write(keyvalue);

	// convert GDK keyvalue into HUI key id
	key = -1;
	for (int i=0;i<HUI_NUM_KEYS;i++)
		//if ((HuiKeyID[i] == keyvalue)||(HuiKeyID2[i] == keyvalue))
		if (HuiKeyID[i] == keyvalue)
			key = i;
	key_code = key;
	if (key < 0)
		return;


	// key code?
	if ((event->state & GDK_CONTROL_MASK) > 0)
		key_code += KEY_CONTROL;
	if ((event->state & GDK_SHIFT_MASK) > 0)
		key_code += KEY_SHIFT;
	if (((event->state & GDK_MOD1_MASK) > 0) /*|| ((event->state & GDK_MOD2_MASK) > 0) || ((event->state & GDK_MOD5_MASK) > 0)*/)
		key_code += KEY_ALT;
}

bool area_process_key(GdkEventKey *event, HuiControlDrawingArea *c, bool down)
{
	int key, key_code;
	_get_hui_key_id_(event, key, key_code);
	if (key < 0)
		return false;

	c->win->input.key_code = key;

	if (down){
		c->win->input.key_code = key_code;
	}

	c->Notify(down ? "hui:key-down" : "hui:key-up", false);

	return c->grab_focus;
}



gboolean OnGtkAreaKeyDown(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	return area_process_key(event, (HuiControlDrawingArea*)user_data, true);
}

gboolean OnGtkAreaKeyUp(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	return area_process_key(event, (HuiControlDrawingArea*)user_data, false);
}

HuiControlDrawingArea::HuiControlDrawingArea(const string &title, const string &id) :
	HuiControl(HuiKindDrawingArea, id)
{
	GetPartStrings(id, title);
	GtkWidget *da = gtk_drawing_area_new();
	g_signal_connect(G_OBJECT(da), "draw", G_CALLBACK(OnGtkAreaDraw), this);
	g_signal_connect(G_OBJECT(da), "key-press-event", G_CALLBACK(&OnGtkAreaKeyDown), this);
	g_signal_connect(G_OBJECT(da), "key-release-event", G_CALLBACK(&OnGtkAreaKeyUp), this);
	//g_signal_connect(G_OBJECT(da), "size-request", G_CALLBACK(&OnGtkAreaResize), this);
	g_signal_connect(G_OBJECT(da), "motion-notify-event", G_CALLBACK(&OnGtkAreaMouseMove), this);
	g_signal_connect(G_OBJECT(da), "button-press-event", G_CALLBACK(&OnGtkAreaButtonDown), this);
	g_signal_connect(G_OBJECT(da), "button-release-event", G_CALLBACK(&OnGtkAreaButtonUp), this);
	g_signal_connect(G_OBJECT(da), "scroll-event", G_CALLBACK(&OnGtkAreaMouseWheel), this);
	//g_signal_connect(G_OBJECT(w), "focus-in-event", G_CALLBACK(&focus_in_event), this);
	int mask;
	g_object_get(G_OBJECT(da), "events", &mask, NULL);
	mask |= GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_VISIBILITY_NOTIFY_MASK | GDK_SCROLL_MASK; // GDK_POINTER_MOTION_HINT_MASK = "fewer motions"
	//mask = GDK_ALL_EVENTS_MASK;
	g_object_set(G_OBJECT(da), "events", mask, NULL);

	grab_focus = (OptionString.find("grabfocus") >= 0);
	if (grab_focus){
		gtk_widget_set_can_focus(da, true);
		gtk_widget_grab_focus(da);
	}
	widget = da;
	gtk_widget_set_hexpand(widget, true);
	gtk_widget_set_vexpand(widget, true);
	SetOptions(OptionString);
}

HuiControlDrawingArea::~HuiControlDrawingArea() {
}

#endif
