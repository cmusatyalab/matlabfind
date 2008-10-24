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
#include <string.h>

#include "matlabfind.h"
#include "define.h"
#include "util.h"

static GList *simulated_circles;

GtkListStore *saved_search_store;
static GdkPixbuf *c_pix_scaled;
static float scale;

static gdouble current_sharpness;

static void list_deleter(gpointer data, gpointer user_data) {
  g_free(data);
}

void reset_sharpness(void) {
  // set to 1
  current_sharpness = 1;
  gtk_range_set_value(GTK_RANGE(glade_xml_get_widget(g_xml,
						     "minSharpness")),
		      current_sharpness);
  gtk_widget_set_sensitive(glade_xml_get_widget(g_xml, "recomputePreview"),
			   FALSE);
}

void draw_define_offscreen_items(gint a_w, gint a_h) {
    // clear old scaled pix
  if (c_pix_scaled != NULL) {
    g_object_unref(c_pix_scaled);
    c_pix_scaled = NULL;
  }

  // reset sharpness
  reset_sharpness();

#if 0
  // if something selected?
  if (c_pix) {
    float p_aspect =
      (float) gdk_pixbuf_get_width(c_pix) /
      (float) gdk_pixbuf_get_height(c_pix);

    float w = a_w;
    float h = a_h;
    float w_aspect = (float) w / (float) h;

    /* is window wider than pixbuf? */
    if (p_aspect < w_aspect) {
      /* then calculate width from height */
      w = h * p_aspect;
      scale = (float) a_h
	/ (float) gdk_pixbuf_get_height(c_pix);
    } else {
      /* else calculate height from width */
      h = w / p_aspect;
      scale = (float) a_w
	/ (float) gdk_pixbuf_get_width(c_pix);
    }

    c_pix_scaled = gdk_pixbuf_scale_simple(c_pix,
					   w, h,
					   GDK_INTERP_BILINEAR);
  }
#endif

}

void on_saveSearchButton_clicked (GtkButton *button,
				  gpointer   user_data) {
  GtkTreeIter iter;

  const gchar *save_name =
    gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(g_xml, "searchName")));

  const gchar *eval_fn =
    gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(g_xml, "evalFunction")));

  const gchar *init_fn =
    gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(g_xml, "initFunction")));

  const gchar *src_folder =
    gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(glade_xml_get_widget(g_xml, "srcFolder")));

  const gchar *threshold_str =
    gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(g_xml, "threshold")));

  gdouble threshold = g_ascii_strtod(threshold_str, NULL);

  g_debug("making new search: %s", save_name);
  gtk_list_store_append(saved_search_store, &iter);
  gtk_list_store_set(saved_search_store, &iter,
		     0, save_name,
		     1, eval_fn,
		     2, init_fn,
		     3, threshold,
		     4, src_folder,
		     -1);

  // don't allow double click
  gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
}

void on_searchName_changed (GtkEditable *editable,
			    gpointer     user_data) {
  // change the sensitivity of the save as button, to allow clicking
  // only when the user has changed to non-empty string
  gchar *text = gtk_editable_get_chars(editable, 0, -1);
  gboolean empty = (strlen(text) == 0);
  g_free(text);

  gtk_widget_set_sensitive(GTK_WIDGET(glade_xml_get_widget(g_xml, "saveSearchButton")),
			   !empty);
}
