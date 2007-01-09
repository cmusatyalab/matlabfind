/*
 * MATLABFind: A Diamond application for interoperating with MATLAB
 *
 * Copyright (c) 2006-2007 Carnegie Mellon University. All rights reserved.
 * Additional copyrights may be listed below.
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License v1.0 which accompanies this
 * distribution in the file named LICENSE.
 *
 * Technical and financial contributors are listed in the file named
 * CREDITS.
 */

#ifndef DEFINE_H
#define DEFINE_H

#include <gtk/gtk.h>

void draw_define_offscreen_items (gint width, gint height);
void reset_sharpness(void);
void copy_to_simulated_circles(GList *circles);

gboolean on_simulatedSearch_expose_event (GtkWidget *d,
					  GdkEventExpose *event,
					  gpointer user_data);

gboolean on_simulatedSearch_configure_event (GtkWidget         *widget,
					     GdkEventConfigure *event,
					     gpointer          user_data);

void on_define_search_value_changed (GtkRange *range,
				     gpointer  user_data);

void on_saveSearchButton_clicked (GtkButton *button,
				  gpointer   user_data);

void on_minSharpnessvalue_changed (GtkRange *range,
				   gpointer  user_data);

void on_recomputePreview_clicked (GtkButton *button,
				  gpointer   user_data);

void on_searchName_changed (GtkEditable *editable,
			    gpointer     user_data);
#endif
