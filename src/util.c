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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "util.h"

#include <math.h>

gdouble compute_eccentricity(gdouble a, gdouble b) {
  if (b > a) {
    gdouble c = a;
    a = b;
    b = c;
  }

  return sqrt(1 - ((b*b) / (a*a)));
}


gdouble quadratic_mean_radius(gdouble a, gdouble b) {
  gdouble result;

  if (b > a) {
    gdouble c = a;
    a = b;
    b = c;
  }

  result = sqrt((3.0*a*a + b*b) / 4.0);

  return result;
}
