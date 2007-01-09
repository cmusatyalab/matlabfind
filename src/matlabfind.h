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

#ifndef MATLABFIND_H
#define MATLABFIND_H

#include <glade/glade.h>
#include <gtk/gtk.h>

extern GladeXML *g_xml;
extern GdkPixbuf *c_pix;


extern GtkListStore *saved_search_store;

extern GtkListStore *found_items;


#endif
