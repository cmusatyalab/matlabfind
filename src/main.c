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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "matlabfind.h"
#include "diamond_interface.h"

static void setup_saved_search_store(void) {
  GtkTreeView *v = GTK_TREE_VIEW(glade_xml_get_widget(g_xml,
						      "definedSearches"));
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  saved_search_store = gtk_list_store_new(5,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_STRING,
					  G_TYPE_DOUBLE,
					  G_TYPE_STRING);

  gtk_tree_view_set_model(v, GTK_TREE_MODEL(saved_search_store));


  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes ("Name",
						     renderer,
						     "text", 0,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (v), column);
}

static void setup_results_store(GtkIconView *g) {
  GtkTreeIter iter;

  found_items =
    gtk_list_store_new(3,
		       GDK_TYPE_PIXBUF,
		       G_TYPE_STRING,
		       GDK_TYPE_PIXBUF);

  gtk_icon_view_set_model(g, GTK_TREE_MODEL(found_items));
  gtk_icon_view_set_pixbuf_column(g, 0);
  gtk_icon_view_set_text_column(g, 1);
}


void on_about1_activate (GtkMenuItem *menuitem, gpointer user_data) {
  gtk_widget_show(glade_xml_get_widget(g_xml, "aboutdialog1"));
}


GladeXML *g_xml;

int
main (int argc, char *argv[])
{
  GtkWidget *matlabfind;

  gtk_init(&argc,&argv);
  g_xml = glade_xml_new(MATLABFIND_GLADEDIR "/matlabfind.glade",NULL,NULL);
  g_assert(g_xml != NULL);

  glade_xml_signal_autoconnect(g_xml);
  matlabfind = glade_xml_get_widget(g_xml,"matlabfind");

  // init saved searches
  setup_saved_search_store();

  // init results
  setup_results_store(GTK_ICON_VIEW(glade_xml_get_widget(g_xml,
							 "searchResults")));
  // init diamond
  diamond_init();

  gtk_widget_show_all(matlabfind);

  gtk_main();
  return 0;
}
