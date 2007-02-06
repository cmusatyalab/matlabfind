/*
 *  MATLABFind
 *  A Diamond application for interoperating with MATLAB
 *  Version 1
 *
 *  Copyright (c) 2006-2007 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef INVESTIGATE_H
#define INVESTIGATE_H

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif
  void on_clearSearch_clicked (GtkButton *button,
			       gpointer   user_data);

  void on_stopSearch_clicked (GtkButton *button,
			      gpointer   user_data);

  void on_startSearch_clicked (GtkButton *button,
			       gpointer   user_data);

  gboolean on_selectedResult_expose_event (GtkWidget *d,
					   GdkEventExpose *event,
					   gpointer user_data);

  gboolean on_selectedResult_button_press_event (GtkWidget      *widget,
						 GdkEventButton *event,
						 gpointer        user_data);

  gboolean on_selectedResult_button_release_event (GtkWidget      *widget,
						   GdkEventButton *event,
						   gpointer        user_data);

  gboolean on_selectedResult_motion_notify_event (GtkWidget      *widget,
						  GdkEventMotion *event,
						  gpointer        user_data);

  gboolean on_selectedResult_configure_event (GtkWidget         *widget,
					      GdkEventConfigure *event,
					      gpointer          user_data);

  void on_searchResults_selection_changed (GtkIconView *view,
					   gpointer user_data);

  gboolean on_selectedResult_enter_notify_event (GtkWidget        *widget,
						 GdkEventCrossing *event,
						 gpointer          user_data);

  gboolean on_selectedResult_leave_notify_event (GtkWidget        *widget,
						 GdkEventCrossing *event,
						 gpointer          user_data);

  void on_generateHistogram_clicked (GtkButton *button,
				     gpointer user_data);
#ifdef __cplusplus
}
#endif

#endif
