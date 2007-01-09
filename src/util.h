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
