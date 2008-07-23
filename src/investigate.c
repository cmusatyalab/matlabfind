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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <stdlib.h>
#include <math.h>

#include "matlabfind.h"
#include "investigate.h"
#include "util.h"
#include "diamond_interface.h"

GtkListStore *found_items;

static GdkPixbuf *i_pix;
static GdkPixbuf *i_pix_scaled;

static gdouble display_scale = 1.0;

static ls_search_handle_t dr;
static guint search_idle_id;

static GtkTreePath *current_result_path;

static void stop_search(void) {
  if (dr != NULL) {
    printf("terminating search\n");
    g_source_remove(search_idle_id);
    ls_terminate_search(dr);
    dr = NULL;
  }
}

static void foreach_select_investigation(GtkIconView *icon_view,
					 GtkTreePath *path,
					 gpointer data) {
  GError *err = NULL;

  GtkTreeIter iter;
  GtkTreeModel *m = gtk_icon_view_get_model(icon_view);
  GtkWidget *w;

  // save path
  if (current_result_path != NULL) {
    gtk_tree_path_free(current_result_path);
  }
  current_result_path = gtk_tree_path_copy(path);

  gtk_tree_model_get_iter(m, &iter, path);
  gtk_tree_model_get(m, &iter,
		     2, &i_pix,
		     -1);

  w = glade_xml_get_widget(g_xml, "selectedResult");
  gtk_widget_queue_draw(w);
}


static void draw_investigate_offscreen_items(gint allocation_width,
					     gint allocation_height) {
  // clear old scaled pix
  if (i_pix_scaled != NULL) {
    g_object_unref(i_pix_scaled);
    i_pix_scaled = NULL;
  }

  // if something selected?
  if (i_pix) {
    GdkGC *gc;

    float p_aspect =
      (float) gdk_pixbuf_get_width(i_pix) /
      (float) gdk_pixbuf_get_height(i_pix);
    int w = allocation_width;
    int h = allocation_height;
    float w_aspect = (float) w / (float) h;

    /* is window wider than pixbuf? */
    if (p_aspect < w_aspect) {
      /* then calculate width from height */
      w = (int) (h * p_aspect);
      display_scale = (float) allocation_height
	/ (float) gdk_pixbuf_get_height(i_pix);
    } else {
      /* else calculate height from width */
      h = (int) (w / p_aspect);
      display_scale = (float) allocation_width
	/ (float) gdk_pixbuf_get_width(i_pix);
    }

    i_pix_scaled = gdk_pixbuf_scale_simple(i_pix,
					   w, h,
					   GDK_INTERP_BILINEAR);

  }
}



void on_clearSearch_clicked (GtkButton *button,
			     gpointer   user_data) {
  // buttons
  GtkWidget *stopSearch = glade_xml_get_widget(g_xml, "stopSearch");
  GtkWidget *startSearch = glade_xml_get_widget(g_xml, "startSearch");

  gtk_widget_set_sensitive(stopSearch, FALSE);
  gtk_widget_set_sensitive(startSearch, TRUE);

  // stop
  stop_search();

  // clear search thumbnails
  gtk_list_store_clear(found_items);

  // clear result
  if (i_pix != NULL) {
    g_object_unref(i_pix);
    i_pix = NULL;
  }
  gtk_widget_queue_draw(glade_xml_get_widget(g_xml, "selectedResult"));
}

void on_stopSearch_clicked (GtkButton *button,
			    gpointer user_data) {
  GtkWidget *stopSearch = glade_xml_get_widget(g_xml, "stopSearch");
  GtkWidget *startSearch = glade_xml_get_widget(g_xml, "startSearch");
  gtk_widget_set_sensitive(stopSearch, FALSE);
  gtk_widget_set_sensitive(startSearch, TRUE);

  stop_search();
}

void on_startSearch_clicked (GtkButton *button,
			     gpointer   user_data) {
  // get the selected search
  GtkTreeIter s_iter;
  GtkTreeSelection *selection =
    gtk_tree_view_get_selection(GTK_TREE_VIEW(glade_xml_get_widget(g_xml,
								   "definedSearches")));
  GtkTreeModel *model = GTK_TREE_MODEL(saved_search_store);

  g_debug("selection: %p", selection);

  if (gtk_tree_selection_get_selected(selection,
				      &model,
				      &s_iter)) {
    gchar *eval_fn;
    gchar *init_fn;
    gdouble threshold;
    gchar *src_folder;
    GtkWidget *stopSearch = glade_xml_get_widget(g_xml, "stopSearch");


    g_debug("saved_search_store: %p", model);
    gtk_tree_model_get(model,
		       &s_iter,
		       1, &eval_fn,
		       2, &init_fn,
		       3, &threshold,
		       4, &src_folder,
		       -1);

    g_debug("searching with function %s under threshold %lf", eval_fn, threshold);

    // reset stats
    total_objects = 0;
    processed_objects = 0;
    dropped_objects = 0;

    displayed_objects = 0;

    // diamond
    dr = diamond_matlab_search(eval_fn, init_fn,
			       threshold,
			       src_folder);


    // take the handle, put it into the idle callback to get
    // the results?
    search_idle_id = g_timeout_add_full(G_PRIORITY_LOW, 100, diamond_result_callback, dr, NULL);

    // activate the stop search button
    gtk_widget_set_sensitive(stopSearch, TRUE);

    // deactivate our button
    gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

  }
}

gboolean on_selectedResult_expose_event (GtkWidget *d,
					 GdkEventExpose *event,
					 gpointer user_data) {
  if (i_pix) {
    gdk_draw_pixbuf(d->window,
		    d->style->fg_gc[GTK_WIDGET_STATE(d)],
		    i_pix_scaled,
		    0, 0, 0, 0,
		    -1, -1,
		    GDK_RGB_DITHER_NORMAL,
		    0, 0);
  }

  return TRUE;
}


gboolean on_selectedResult_configure_event (GtkWidget         *widget,
					    GdkEventConfigure *event,
					    gpointer          user_data) {
  draw_investigate_offscreen_items(event->width, event->height);
  return TRUE;
}

void on_searchResults_selection_changed (GtkIconView *view,
					 gpointer user_data) {
  GtkWidget *w;

  // load the image
  gtk_icon_view_selected_foreach(view, foreach_select_investigation, NULL);

  // draw the offscreen items
  w = glade_xml_get_widget(g_xml, "selectedResult");
  draw_investigate_offscreen_items(w->allocation.width,
				   w->allocation.height);

}
