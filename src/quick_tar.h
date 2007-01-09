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

#ifndef __QUICK_TAR_H
#define __QUICK_TAR_H

#include <string.h>

static inline char *safe_strncpy(char *dest, const char *src, size_t n)
{
   char *ret = strncpy(dest, src, n);

   dest[n-1] = '\0';

   return ret;
}

int untar_blob(const char *src_dir_name, unsigned int bloblen, char *blob_data);
int tar_blob(const char *src_dir_name, int fd);

#endif
