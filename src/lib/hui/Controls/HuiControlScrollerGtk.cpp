/*
 * HuiControlScrollerGtk.cpp
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#include "HuiControlScroller.h"

#ifdef HUI_API_GTK

HuiControlScroller::HuiControlScroller(const string &title, const string &id) :
	HuiControl(HuiKindScroller, id)
{
	widget = gtk_scrolled_window_new(NULL, NULL);
	viewport = gtk_viewport_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(widget), viewport);
	gtk_widget_show(viewport);
}

HuiControlScroller::~HuiControlScroller()
{
}

void HuiControlScroller::add(HuiControl *child, int x, int y)
{
	GtkWidget *child_widget = child->widget;
	if (child->frame)
		child_widget = child->frame;
	gtk_container_add(GTK_CONTAINER(viewport), child_widget);
}

#endif
