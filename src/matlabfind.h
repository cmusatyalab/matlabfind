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

#ifndef MATLABFIND_H
#define MATLABFIND_H

#include <glade/glade.h>
#include <gtk/gtk.h>

extern GladeXML *g_xml;
extern GdkPixbuf *c_pix;


extern GtkListStore *saved_search_store;

extern GtkListStore *found_items;


#endif
