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

#include <glib.h>

#include "diamond_interface.h"
#include "matlabfind.h"
#include "util.h"
#include "quick_tar.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <unistd.h>
#include <sys/mman.h>

#include "lib_filter.h"
#include "lib_dconfig.h"

static struct collection_t collections[MAX_ALBUMS+1] = { { NULL } };
static gid_list_t diamond_gid_list;

int total_objects;
int processed_objects;
int dropped_objects;

int displayed_objects;

static void update_gids(void) {
  int i, j;

  // clear
  diamond_gid_list.num_gids = 0;
  for (i=0; i<MAX_ALBUMS && collections[i].name; i++) {
    if (collections[i].active) {
      int err;
      int num_gids = MAX_ALBUMS;
      groupid_t gids[MAX_ALBUMS];
      err = nlkup_lookup_collection(collections[i].name, &num_gids, gids);
      g_assert(!err);
      for (j=0; j < num_gids; j++) {
	printf("adding gid: %lld for collection %s\n", gids[j], collections[i].name);
	diamond_gid_list.gids[diamond_gid_list.num_gids++] = gids[j];
      }
    }
  }
}

void diamond_init(void) {
  int i;
  int j;
  int err;
  void *cookie;
  char *name;

  printf("reading collections...\n");
  {
    int pos = 0;

    err = nlkup_first_entry(&name, &cookie);
    while(!err && pos < MAX_ALBUMS) {

      printf(" collection %2d: %s\n", pos, name);
      collections[pos].name = name;
      collections[pos].active = (pos == 0) ? 1 : 0;

      pos++;
      err = nlkup_next_entry(&name, &cookie);
    }

    collections[pos].name = NULL;
  }

  update_gids();
}

static ls_search_handle_t generic_search (char *filter_spec_name) {
  ls_search_handle_t diamond_handle;
  int f1, f2;
  int err;

  char buf[1];

  char *rgb_filter_name;

  diamond_handle = ls_init_search();

  err = ls_set_searchlist(diamond_handle, diamond_gid_list.num_gids,
			  diamond_gid_list.gids);
  g_assert(!err);

  // append our stuff
  f1 = g_open(MATLABFIND_FILTERDIR "/rgb-filter.txt", O_RDONLY);
  if (f1 == -1) {
    perror("can't open " MATLABFIND_FILTERDIR "/rgb-filter.txt");
    return NULL;
  }

  f2 = g_open(filter_spec_name, O_WRONLY | O_APPEND);
  if (f2 == -1) {
    printf("can't open %s", filter_spec_name);
    perror("");
    return NULL;
  }

  while (read(f1, buf, 1) > 0) {
    write(f2, buf, 1);   // PERF
  }

  close(f1);
  close(f2);

  // check for snapfind filter
  if (access("/opt/snapfind/lib/fil_rgb.so", F_OK) == 0) {
    rgb_filter_name = "/opt/snapfind/lib/fil_rgb.so";
  } else if (access("/usr/local/diamond/lib/fil_rgb.so", F_OK) == 0) {
    rgb_filter_name = "/usr/local/diamond/lib/fil_rgb.so";
  } else {
    // old way
    rgb_filter_name = MATLABFIND_FILTERDIR "/fil_rgb.so";
  }

  err = ls_set_searchlet(diamond_handle, DEV_ISA_IA32,
			 rgb_filter_name,
			 filter_spec_name);
  g_assert(!err);


  return diamond_handle;
}


static void update_stats(ls_search_handle_t dr) {
  int num_dev;
  ls_dev_handle_t dev_list[16];
  int i, err, len;
  dev_stats_t *dstats;
  int tobj = 0, sobj = 0, dobj = 0;
  GtkLabel *stats_label = GTK_LABEL(glade_xml_get_widget(g_xml, "statsLabel"));

  guchar *tmp;

  dstats = (dev_stats_t *) g_malloc(DEV_STATS_SIZE(32));

  num_dev = 16;

  err = ls_get_dev_list(dr, dev_list, &num_dev);
  if (err != 0) {
    g_error("update stats: %d", err);
  }

  for (i = 0; i < num_dev; i++) {
    len = DEV_STATS_SIZE(32);

    err = ls_get_dev_stats(dr, dev_list[i], dstats, &len);
    if (err) {
      g_error("Failed to get dev stats");
    }
    tobj += dstats->ds_objs_total;
    sobj += dstats->ds_objs_processed;
    dobj += dstats->ds_objs_dropped;
  }
  g_free(dstats);

  total_objects = tobj;
  processed_objects = sobj;
  dropped_objects = dobj;
  tmp =
    g_strdup_printf("Total objects: %d, Processed objects: %d, Dropped objects: %d, Displayed objects: %d",
		    total_objects, processed_objects, dropped_objects, displayed_objects);
  gtk_label_set_text(stats_label, tmp);
  g_free(tmp);
}


static void compute_thumbnail_scale(double *scale, gint *w, gint *h) {
  float p_aspect = (float) *w / (float) *h;

  if (p_aspect < 1) {
    /* then calculate width from height */
    *scale = 150.0 / (double) *h;
    *h = 150;
    *w = *h * p_aspect;
  } else {
    /* else calculate height from width */
    *scale = 150.0 / (double) *w;
    *w = 150;
    *h = *w / p_aspect;
  }
}

gboolean diamond_result_callback(gpointer g_data) {
  // this gets 0 or 1 result from diamond and puts it into the
  // icon view

  ls_obj_handle_t obj;
  void *data;
  char *name;
  void *cookie;
  double matlab_ans;
  unsigned int len;
  int err;
  int w, origW;
  int h, origH;
  GdkPixbuf *pix, *pix_copy, *pix2;

  static time_t last_time;

  int i;

  GList *clist = NULL;
  gchar *title;

  GtkTreeIter iter;

  double scale;

  ls_search_handle_t dr;


  // get handle
  dr = (ls_search_handle_t) g_data;

  // get handle
  err = ls_next_object(dr, &obj, LSEARCH_NO_BLOCK);

  // XXX should be able to use select()
  if (err == EWOULDBLOCK) {
    // no results right now
    time_t now = time(NULL);
    if (now > last_time) {
      update_stats(dr);
    }
    last_time = now;

    return TRUE;
  } else if (err || (displayed_objects > 100)) {
    // no more results
    GtkWidget *stopSearch = glade_xml_get_widget(g_xml, "stopSearch");
    GtkWidget *startSearch = glade_xml_get_widget(g_xml, "startSearch");
    gtk_widget_set_sensitive(stopSearch, FALSE);
    gtk_widget_set_sensitive(startSearch, TRUE);

    update_stats(dr);

    ls_abort_search(dr);
    return FALSE;
  }

  // more results
  printf("got object: %p\n", obj);

  displayed_objects++;

  // thumbnail
  err = lf_ref_attr(obj, "_cols.int", &len, (unsigned char **) &data);
  g_assert(!err);
  origW = w = *((int *) data);

  err = lf_ref_attr(obj, "_rows.int", &len, (unsigned char **) &data);
  g_assert(!err);
  origH = h = *((int *) data);

  err = lf_ref_attr(obj, "_rgb_image.rgbimage", &len, (unsigned char **) &data);
  g_assert(!err);

  printf(" img %dx%d (%d bytes)\n", w, h, len);

  pix = gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB,
				 TRUE, 8, w, h, w*4, NULL, NULL);
  pix_copy = gdk_pixbuf_copy(pix);


  compute_thumbnail_scale(&scale, &w, &h);

  // draw into thumbnail
  pix2 = gdk_pixbuf_scale_simple(pix, w, h, GDK_INTERP_BILINEAR);

  err = lf_ref_attr(obj, "_matlab_ans.double", &len, (unsigned char **) &data);
  g_assert(!err);
  matlab_ans = *((double *) data);
  title = g_strdup_printf("%g", matlab_ans);

  // store
  gtk_list_store_append(found_items, &iter);
  gtk_list_store_set(found_items, &iter,
		     0, pix2,
		     1, title,
		     2, pix_copy,
		     -1);

  g_object_unref(pix);
  g_object_unref(pix_copy);
  g_object_unref(pix2);


  //  err = lf_first_attr(obj, &name, &len, &data, &cookie);
  //  while (!err) {
  //    printf(" attr name: %s, length: %d\n", name, len);
  //    err = lf_next_attr(obj, &name, &len, &data, &cookie);
  //  }

  err = ls_release_object(dr, obj);
  g_assert(!err);

  return TRUE;
}


ls_search_handle_t diamond_matlab_search(const gchar *eval_fn, const gchar *init_fn,
					 gdouble threshold, const gchar *src_folder) {
  ls_search_handle_t dr;
  int fd, blob_fd;
  FILE *f;
  gchar *name_used, *blob_name_used;
  int err, blob_len;
  void *blob_data;

  // temporary file
  fd = g_file_open_tmp(NULL, &name_used, NULL);
  g_assert(fd >= 0);

  blob_fd = g_file_open_tmp(NULL, &blob_name_used, NULL);
  g_assert(fd >= 0);

  blob_len = tar_blob(src_folder + 7, blob_fd);
  g_assert(blob_len >= 0);

  blob_data = mmap(NULL, blob_len, PROT_READ, MAP_PRIVATE, blob_fd, 0);
  g_assert(blob_data != MAP_FAILED); 

  // write out file for matlab search
  f = fdopen(fd, "a");
  g_return_val_if_fail(f, NULL);
  fprintf(f, "\n\n"
	  "FILTER matlab_exec\n"
	  "THRESHOLD %d\n"
	  "EVAL_FUNCTION  f_eval_matlab_exec\n"
	  "INIT_FUNCTION  f_init_matlab_exec\n"
	  "FINI_FUNCTION  f_fini_matlab_exec\n"
	  "ARG  %s\n" // init function
	  "ARG  %s\n" // eval function
	  "REQUIRES  RGB # dependencies\n"
	  "MERIT  50 # some relative cost\n",
	  (int)threshold, init_fn, eval_fn);
  fflush(f);

  // initialize with generic search
  dr = generic_search(name_used);

  // add filter
  err = ls_add_filter_file(dr, DEV_ISA_IA32,
			   MATLABFIND_FILTERDIR "/fil_matlab_exec.so");
  g_assert(!err);

  err = ls_set_blob(dr, "matlab_exec", blob_len, blob_data);
  g_assert(!err);

  // now close
  munmap(blob_data, blob_len);
  close(blob_fd);
  fclose(f);
  g_free(name_used);
  g_free(blob_name_used);

  // start search
  ls_start_search(dr);

  // return
  return dr;
}

