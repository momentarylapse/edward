/*
 * HuiControlExpanderGtk.cpp
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#include "HuiControlExpander.h"

#ifdef HUI_API_GTK

HuiControlExpander::HuiControlExpander(const string &title, const string &id) :
	HuiControl(HuiKindExpander, id)
{
	GetPartStrings(id, title);
	widget = gtk_expander_new(sys_str(PartString[0]));
}

HuiControlExpander::~HuiControlExpander()
{
}

void HuiControlExpander::add(HuiControl *child, int x, int y)
{
	GtkWidget *child_widget = child->widget;
	if (child->frame)
		child_widget = child->frame;
	gtk_widget_set_vexpand(child_widget, true);
	gtk_widget_set_hexpand(child_widget, true);
	gtk_container_add(GTK_CONTAINER(widget), child_widget);
}

#endif
