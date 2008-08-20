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

#ifndef __QUICK_TAR_H
#define __QUICK_TAR_H

#include <string.h>

static inline char *safe_strncpy(char *dest, const char *src, size_t n)
{
   char *ret = strncpy(dest, src, n);

   dest[n-1] = '\0';

   return ret;
}


#ifdef __cplusplus 
extern "C" {
#endif

int untar_blob(const char *src_dir_name, unsigned int bloblen, char *blob_data);
int tar_blob(const char *src_dir_name, int fd);

#ifdef __cplusplus
}
#endif


#endif
