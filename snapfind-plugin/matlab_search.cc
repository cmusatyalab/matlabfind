/*
 *  MATLABFind
 *  A Diamond application for interoperating with MATLAB
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  Copyright (c) 2008, 2010 Carnegie Mellon University
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
#include <glib.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/queue.h>
#include "lib_results.h"
#include "rgb.h"
#include "matlab_search.h"
#include "factory.h"
#include "quick_tar.h"

#define	MAX_DISPLAY_NAME	64

/* config file tokens that we write out */
#define SEARCH_NAME     "matlab_search"
#define EVAL_FUNCTION_ID    "EVAL_FUNCTION"
#define INIT_FUNCTION_ID    	"INIT_FUNCTION"
#define THRESHOLD_ID "THRESHOLD"
#define SOURCE_FOLDER_ID 	"SOURCE_FOLDER"



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
	matlab_factory *fac;
	fac = new matlab_factory;
	matlab_codec_factory *fac2;
	fac2 = new matlab_codec_factory;
	factory_register(fac);
	factory_register_codec(fac2);  // also does codec
}



matlab_search::matlab_search(const char *name, char *descr)
		: img_search(name, descr)
{
	eval_function = g_strdup("eval");
	init_function = g_strdup("init");
	threshold = g_strdup("0");
	source_folder = NULL;

	edit_window = NULL;

	return;
}

matlab_search::~matlab_search()
{
	if (eval_function) {
		g_free(eval_function);
	}
	if (init_function) {
		g_free(init_function);
	}
	if (threshold) {
		g_free(threshold);
	}
	if (source_folder) {
		g_free(source_folder);
	}

	g_free(get_auxiliary_data());
	return;
}


int
matlab_search::handle_config(int nconf, char **data)
{
	int	err;

	if (strcmp(EVAL_FUNCTION_ID, data[0]) == 0) {
		assert(nconf > 1);
		eval_function = g_strdup(data[1]);
		assert(eval_function != NULL);
		err = 0;
	} else if (strcmp(INIT_FUNCTION_ID, data[0]) == 0) {
		assert(nconf > 1);
		init_function = g_strdup(data[1]);
		assert(init_function != NULL);
		err = 0;
	} else if (strcmp(THRESHOLD_ID, data[0]) == 0) {
		assert(nconf > 1);
		threshold = g_strdup(data[1]);
		assert(threshold != NULL);
		err = 0;
	} else if (strcmp(SOURCE_FOLDER_ID, data[0]) == 0) {
		assert(nconf > 1);
		source_folder = g_strdup(data[1]);
		assert(source_folder != NULL);
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
	matlab_search *    search;
	search = (matlab_search *)data;
	search->close_edit_win();
}

static void
cb_choose_folder(GtkFileChooser* item, gpointer data)
{
	matlab_search *    search;
	search = (matlab_search *)data;
	search->populate_function_menus();
}

void
matlab_search::populate_function_menus()
{
	char *uri;
	char *dirpath;
	GDir *dir;
	const gchar *name;
	char *path;
	GList *names = NULL;
	GList *cur;
	char *initial_init_func;
	char *initial_eval_func;
	char *tmp;

	/* Determine which items to select initially */
	initial_init_func = gtk_combo_box_get_active_text(GTK_COMBO_BOX(init_function_menu));
	if (initial_init_func == NULL || !strcmp("", initial_init_func)) {
		g_free(initial_init_func);
		initial_init_func = g_strdup(init_function);
	}
	initial_eval_func = gtk_combo_box_get_active_text(GTK_COMBO_BOX(eval_function_menu));
	if (initial_eval_func == NULL) {
		initial_eval_func = g_strdup(eval_function);
	}

	/* Purge existing menus */
	for (; function_menu_length; function_menu_length--) {
		gtk_combo_box_remove_text(GTK_COMBO_BOX(init_function_menu), 0);
		gtk_combo_box_remove_text(GTK_COMBO_BOX(eval_function_menu), 0);
	}
	/* Init menu has an extra entry */
	gtk_combo_box_remove_text(GTK_COMBO_BOX(init_function_menu), 0);

	/* Gather a list of files in the source folder */
	uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(source_folder_button));
	dirpath = uri ? g_filename_from_uri(uri, NULL, NULL) : NULL;
	g_free(uri);
	dir = g_dir_open(dirpath, 0, NULL);
	if (dir != NULL) {
		while ((name = g_dir_read_name(dir))) {
			path = g_strdup_printf("%s/%s", dirpath, name);
			if (name[0] != '.' && g_file_test(path, G_FILE_TEST_IS_REGULAR) &&
			    g_str_has_suffix(name, ".m")) {
				tmp = g_strdup(name);
				g_strrstr(tmp, ".m")[0] = 0;
				names = g_list_prepend(names, tmp);
			}
			g_free(path);
		}
		g_dir_close(dir);
	}
	g_free(dirpath);
	names = g_list_sort(names, (GCompareFunc) strcmp);

	/* Populate the menus */
	gtk_combo_box_append_text(GTK_COMBO_BOX(init_function_menu), "");
	gtk_combo_box_set_active(GTK_COMBO_BOX(init_function_menu), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(eval_function_menu), -1);
	for (cur = g_list_first(names); cur; cur = g_list_next(cur)) {
		tmp = (char *) cur->data;
		gtk_combo_box_append_text(GTK_COMBO_BOX(init_function_menu), tmp);
		gtk_combo_box_append_text(GTK_COMBO_BOX(eval_function_menu), tmp);
		if (initial_init_func != NULL && !strcmp(tmp, initial_init_func)) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(init_function_menu), function_menu_length + 1);
		}
		if (initial_eval_func != NULL && !strcmp(tmp, initial_eval_func)) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(eval_function_menu), function_menu_length);
		}
		function_menu_length++;
		g_free(tmp);
	}
	g_list_free(names);
	g_free(initial_init_func);
	g_free(initial_eval_func);
}

void
matlab_search::edit_search()
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

	/* set the first row label and file chooser button for the source directory */
	widget = gtk_label_new("Source folder");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 0, 1);
	source_folder_button = gtk_file_chooser_button_new("Select a Folder",
							   GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	/* rhbz #638495 */
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(source_folder_button),
					 FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table), source_folder_button, 1, 2, 0, 1);
	g_signal_connect(G_OBJECT(source_folder_button), "selection-changed",
	                 G_CALLBACK(cb_choose_folder), this);
	if (source_folder != NULL) {
		gtk_file_chooser_set_uri(GTK_FILE_CHOOSER(source_folder_button), source_folder);
	}

	/* set the second row label and popup menu for the init function */
	widget = gtk_label_new("Init function");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 1, 2);
	init_function_menu = gtk_combo_box_new_text();
	gtk_table_attach_defaults(GTK_TABLE(table), init_function_menu, 1, 2, 1, 2);

	/* set the third row label and popup menu for the eval function */
	widget = gtk_label_new("Eval function");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 2, 3);
	eval_function_menu = gtk_combo_box_new_text();
	gtk_table_attach_defaults(GTK_TABLE(table), eval_function_menu, 1, 2, 2, 3);
	function_menu_length = 0;
	populate_function_menus();

	/* set the fourth row label and text entry for the threshold */
	widget = gtk_label_new("Threshold");
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 3, 4);
	threshold_entry = gtk_entry_new();
	gtk_table_attach_defaults(GTK_TABLE(table), threshold_entry, 1, 2, 3, 4);
	if (threshold != NULL) {
		gtk_entry_set_text(GTK_ENTRY(threshold_entry), threshold);
	}

	/* make everything visible */
	gtk_widget_show_all(edit_window);

	return;
}



/*
 * This method reads the values from the current edit
 * window if there is an active one.
 */

void
matlab_search::save_edits()
{
	int fd;
	int blob_len;
	gchar *name_used;
	gboolean success;
	gchar *blob_data;
	gchar *tmp;

	if (edit_window == NULL) {
		return;
	}

	if (eval_function != NULL) {
		g_free(eval_function);
	}
	if (init_function != NULL) {
		g_free(init_function);
	}
	if (threshold != NULL) {
		g_free(threshold);
	}
	if (source_folder != NULL) {
		g_free(source_folder);
	}

	eval_function = gtk_combo_box_get_active_text(GTK_COMBO_BOX(eval_function_menu));
	if (eval_function == NULL) {
		eval_function = g_strdup("1");
	}
	init_function = gtk_combo_box_get_active_text(GTK_COMBO_BOX(init_function_menu));
	if (!strcmp(init_function, "")) {
		g_free(init_function);
		init_function = g_strdup("1");
	}
	threshold = g_strdup(gtk_entry_get_text(GTK_ENTRY(threshold_entry)));
	assert(threshold != NULL);
	source_folder = g_strdup(gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(source_folder_button)));
	assert(source_folder != NULL);

	/* blob */
	g_free(get_auxiliary_data());

	fd = g_file_open_tmp(NULL, &name_used, NULL);
	g_assert(fd >= 0);

	tmp = g_filename_from_uri(source_folder, NULL, NULL);
	blob_len = tar_blob(tmp, fd);
	g_free(tmp);
	g_assert(blob_len >= 0);

	success = g_file_get_contents(name_used, &blob_data, NULL, NULL);
	g_assert(success);

	set_auxiliary_data(blob_data);
	set_auxiliary_data_length(blob_len);

	// save name
	img_search::save_edits();
}


void
matlab_search::close_edit_win()
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
matlab_search::write_fspec(FILE *ostream)
{
	if (strcmp("RGB", get_name()) == 0) {
		fprintf(ostream, "FILTER  RGB\n");
	} else {
		fprintf(ostream, "FILTER  %s  # dependencies \n", get_name());
		fprintf(ostream, "REQUIRES RGB\n");
	}

	fprintf(ostream, "\n");
	fprintf(ostream, "THRESHOLD  %s\n", threshold);
	fprintf(ostream, "MERIT  10000\n");
	fprintf(ostream, "EVAL_FUNCTION  f_eval_matlab_exec  # eval function \n");
	fprintf(ostream, "INIT_FUNCTION  f_init_matlab_exec  # init function \n");
	fprintf(ostream, "FINI_FUNCTION  f_fini_matlab_exec  # fini function \n");
	fprintf(ostream, "ARG  %s\n", init_function );
	fprintf(ostream, "ARG  %s\n", eval_function );
	fprintf(ostream, "\n");
	fprintf(ostream, "\n");
}

void
matlab_search::write_config(FILE *ostream, const char *dirname)
{
 	fprintf(ostream, "SEARCH %s %s\n", SEARCH_NAME, get_name());
 	fprintf(ostream, "%s %s\n", EVAL_FUNCTION_ID, eval_function);
 	fprintf(ostream, "%s %s \n", INIT_FUNCTION_ID, init_function);
 	fprintf(ostream, "%s %s \n", THRESHOLD_ID, threshold);

	if (source_folder != NULL) {
		fprintf(ostream, "%s %s \n", SOURCE_FOLDER_ID, source_folder);
	}
}

/* Region match isn't meaningful for this search */
void
matlab_search::region_match(RGBImage *img, bbox_list_t *blist)
{
	return;
}

bool
matlab_search::is_editable(void)
{
	return true;
}
