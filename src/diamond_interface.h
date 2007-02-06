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

#ifndef DIAMOND_INTERFACE_H
#define DIAMOND_INTERFACE_H

#include "diamond_consts.h"
#include "diamond_types.h"
#include "lib_searchlet.h"

#include <stdio.h>

#define MAX_ALBUMS 32

typedef struct {
  int num_gids;
  groupid_t gids[MAX_ALBUMS];
} gid_list_t;

struct collection_t
{
  char *name;
  //int id;
  int active;
};

extern int total_objects;
extern int processed_objects;
extern int dropped_objects;
extern int displayed_objects;

#ifdef __cplusplus
extern "C" {
#endif
  void diamond_init(void);
  ls_search_handle_t diamond_matlab_search (const gchar *eval_fn,
					    const gchar *init_fn,
					    gdouble threshold,
					    const gchar *src_folder);
  gboolean diamond_result_callback(gpointer data);
#ifdef __cplusplus
}
#endif


#endif
