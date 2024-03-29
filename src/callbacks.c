/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * callbacks.c
 * Copyright (C) Gazy Mahomar 2010 <>
 * 
 * pmdn-dds is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * pmdn-dds is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "callbacks.h"
#include "netlistwin.h"
#include "searchcriteriawindow.h"


void
destroy (GtkWidget *widget, gpointer data)
{
	gtk_main_quit ();
}

void showNL (GtkImageMenuItem *self, gpointer user_data)
{
	GtkWidget *window;

	window = net_list_win_new();
	
	gtk_widget_show_all (window);
}

void showSC (GtkImageMenuItem *self, gpointer user_data)
{
	GtkWidget *window;

	window = search_criteria_window_new();
	
	gtk_widget_show_all (window);
}

