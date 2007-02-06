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

#ifndef UTIL_H
#define UTIL_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

  gdouble compute_eccentricity(gdouble a, gdouble b);
  gdouble quadratic_mean_radius(gdouble a, gdouble b);

#ifdef __cplusplus
}
#endif

#endif
