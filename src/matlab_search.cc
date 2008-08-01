/*
 *  MATLABFind
 *  A Diamond application for interoperating with MATLAB
 *  Version 1
 *
 *  Copyright (c) 2008 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/queue.h>
#include "lib_results.h"
#include "rgb.h"
#include "matlab_search.h"
#include "factory.h"

#define	MAX_DISPLAY_NAME	64

/* config file tokens that we write out */
#define SEARCH_NAME     "text_attr"
#define ATTR_NAME_ID    "ATTR_NAME"
#define STRING_ID    	"STRING"
#define DROP_MISSING_ID "DROP_MISSING"
#define EXACT_MATCH_ID 	"EXACT_MATCH"



extern "C" {
void search_init();
}

/*
 * Initialization function that creates the factory and registers
 * it with the rest of the UI.
 */
void 
search_init()
{
	text_attr_factory *fac;
	fac = new text_attr_factory;
	factory_register(fac);
}



text_attr_search::text_attr_search(const char *name, char *descr)
		: img_search(name, descr)
{
	edit_window = NULL;
	search_string = NULL;
	attr_name = NULL;
	drop_missing = 0;
	exact_match = 1;
	return;
}

text_attr_search::~text_attr_search()
{
	if (search_string) {
		free(search_string);
	}
	if (attr_name) {
		free(attr_name);
	}
	return;
}


int
text_attr_search::handle_config(int nconf, char **data)
{
	int	err;

	if (strcmp(ATTR_NAME_ID, data[0]) == 0) {
		assert(nconf > 1);
		attr_name = strdup(data[1]);
		assert(attr_name != NULL);
		err = 0;

	} else if (strcmp(STRING_ID, data[0]) == 0) {
		assert(nconf > 1);
		search_string = strdup(data[1]);
		assert(search_string != NULL);
		err = 0;
	} else if (strcmp(DROP_MISSING_ID, data[0]) == 0) {
		assert(nconf > 1);
		drop_missing = atoi(data[1]);
		err = 0;
	} else if (strcmp(EXACT_MATCH_ID, data[0]) == 0) {
		assert(nconf > 1);
		exact_match = atoi(data[1]);
		err = 0;
	} else {
		err = img_search::handle_config(nconf, data);
	}

	return(err);
}


static void
cb_edit_done(GtkButton *item, gpointer data)
{
	GtkWidget * widget = (GtkWidget *)data;
	gtk_widget_destroy(widget);
}

static void
cb_close_edit_window(GtkWidget* item, gpointer data)
{
	text_attr_search *    search;
	search = (text_attr_search *)data;
	search->close_edit_win();
}


void
text_attr_search::edit_search()
{
	GtkWidget *     widget;
	GtkWidget *     box;
	GtkWidget *     hbox;
	GtkWidget *     table;
	char        name[MAX_DISPLAY_NAME];

	/* see if it already exists */
	if (edit_window != NULL) {
		/* raise to top ??? */
		gdk_window_raise(GTK_WIDGET(edit_window)->window);
		return;
	}

	edit_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	snprintf(name, MAX_DISPLAY_NAME - 1, "Edit %s", get_name());
	name[MAX_DISPLAY_NAME -1] = '\0';
	gtk_window_set_title(GTK_WINDOW(edit_window), name);
	g_signal_connect(G_OBJECT(edit_window), "destroy",
	                 G_CALLBACK(cb_close_edit_window), this);

	box = gtk_vbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(edit_window), box);

	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, TRUE, 0);

	widget = gtk_button_new_with_label("Close");
	g_signal_connect(G_OBJECT(widget), "clicked",
	                 G_CALLBACK(cb_edit_done), edit_window);
	GTK_WIDGET_SET_FLAGS(widget, GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);

	/*
	 * Get the controls from the img_search.
	 */
	widget = img_search_display();
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);

	/*
 	 * To make the layout look a little cleaner we use a table
	 * to place all the fields.  This will make them be nicely
	 * aligned.
	 */
	table = gtk_table_new(4, 2, FALSE);
        gtk_table_set_row_spacings(GTK_TABLE(table), 2);
        gtk_table_set_col_spacings(GTK_TABLE(table), 4);
        gtk_container_set_border_width(GTK_CONTAINER(table), 10);
	gtk_box_pack_start(GTK_BOX(box), table, FALSE, TRUE, 0);

	/* set the first row label and text entry for the attribute name */
	widget = gtk_label_new("Attribute Name");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 0, 1);
	attr_entry = gtk_entry_new();
	gtk_table_attach_defaults(GTK_TABLE(table), attr_entry, 1, 2, 0, 1);
	if (attr_name != NULL) {
		gtk_entry_set_text(GTK_ENTRY(attr_entry), attr_name);
	}

	/* set the second row label and text entry for the search string */
	widget = gtk_label_new("String");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 1, 2);
	string_entry = gtk_entry_new();
	gtk_table_attach_defaults(GTK_TABLE(table), string_entry, 1, 2, 1, 2);
	if (search_string != NULL) {
		gtk_entry_set_text(GTK_ENTRY(string_entry), search_string);
	}

	/* add label and checkbox to pass/drop objet without named attribute */
	widget = gtk_label_new("Drop missing");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 2, 3);
        drop_cb = gtk_check_button_new();
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(drop_cb), drop_missing);
	gtk_table_attach_defaults(GTK_TABLE(table), drop_cb, 1, 2, 2, 3);


	/* add label and checkbox for exact match vs regex */
        widget = gtk_label_new("Exact Match (not regex)");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 3, 4);
        exact_cb = gtk_check_button_new();
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(exact_cb), exact_match);
	gtk_table_attach_defaults(GTK_TABLE(table), exact_cb, 1, 2, 3, 4);

	/* make everything visible */
	gtk_widget_show_all(edit_window);

	return;
}



/*
 * This method reads the values from the current edit
 * window if there is an active one.
 */

void
text_attr_search::save_edits()
{
	if (edit_window == NULL) {
		return;
	}

	if (search_string != NULL) {
		free(search_string);
	}
	if (attr_name != NULL) {
		free(attr_name);
	}

	search_string = strdup(gtk_entry_get_text(GTK_ENTRY(string_entry)));
	assert(search_string != NULL);
	attr_name = strdup(gtk_entry_get_text(GTK_ENTRY(attr_entry)));
	assert(attr_name != NULL);
	
	drop_missing = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(drop_cb));
	exact_match = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(exact_cb));
	return;
}


void
text_attr_search::close_edit_win()
{
	save_edits();

	/* call parent to give them a chance to cleanup */
	img_search::close_edit_win();

	edit_window = NULL;
}

/*
 * This write the relevant section of the filter specification file
 * for this search.
 */

void
text_attr_search::write_fspec(FILE *ostream)
{

	fprintf(ostream, "\n");
	fprintf(ostream, "FILTER  %s  # dependancies \n", get_name());
	fprintf(ostream, "THRESHOLD  1  # number of hits ?? \n");
	fprintf(ostream, "MERIT  10000  	# guess at cost \n");
	fprintf(ostream, "EVAL_FUNCTION  f_eval_text_attr  # eval function \n");
	fprintf(ostream, "INIT_FUNCTION  f_init_text_attr  # init function \n");
	fprintf(ostream, "FINI_FUNCTION  f_fini_text_attr  # fini function \n");
	fprintf(ostream, "ARG  %s  # attributes to search \n",attr_name );
	fprintf(ostream, "ARG  %s  # string to match \n", search_string );
	fprintf(ostream, "ARG  %d  # exact match  \n", exact_match );
	fprintf(ostream, "ARG  %d  # drop_missing  \n", drop_missing );
	fprintf(ostream, "\n");
	fprintf(ostream, "\n");
}

void
text_attr_search::write_config(FILE *ostream, const char *dirname)
{
 	fprintf(ostream, "SEARCH %s %s\n", SEARCH_NAME, get_name());
 	fprintf(ostream, "%s %s\n", ATTR_NAME_ID, attr_name);
 	fprintf(ostream, "%s %s \n", STRING_ID, search_string);
 	fprintf(ostream, "%s %d \n", DROP_MISSING_ID, drop_missing);
 	fprintf(ostream, "%s %d \n", EXACT_MATCH_ID, exact_match);
}

/* Region match isn't meaninful for this search */
void
text_attr_search::region_match(RGBImage *img, bbox_list_t *blist)
{
	return;
}

