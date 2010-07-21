/*
 *  MATLABFind
 *  A Diamond application for interoperating with MATLAB
 *  Version 1
 *
 *  Copyright (c) 2006-2008 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <glib.h>
#include <stdbool.h>

#include "lib_filter.h"
#include "quick_tar.h"
#include "matlab-compat.h"

#ifndef MATLAB_EXE_PATH
#error Must specify MATLAB_EXE_PATH as compilation flag
#endif


static const int diamond_bytes_per_pixel = 4;
static const int matlab_bytes_per_pixel = 3;

#define matlab_img_name "image"

struct filter_instance {
  char src_dir_name[PATH_MAX];
  char init_matlab_cmd[512];
  char eval_matlab_cmd[512];
  Engine *eng;
  bool is_codec;
};

static void destroy_src_dir(const char *src_dir_name)
{
   DIR *src_dir = opendir(src_dir_name);
   if (src_dir) {
      struct dirent *ent;
      while ( (ent = readdir(src_dir)) ) {
	 char path[PATH_MAX];

	 snprintf(path, sizeof(path), "%s/%s", src_dir_name, ent->d_name);
	 unlink(path);
      }

      closedir(src_dir);
      rmdir(src_dir_name);
   }

}

static Engine *open_engine_in_src_dir(const char *src_dir_name)
{
   Engine *ret;
   char current_path[PATH_MAX];

   if (getcwd(current_path, sizeof(current_path)) < 0) {
      return NULL;
   }

   if (chdir(src_dir_name) < 0) {
      return NULL;
   }

   // try to not have MATLAB screw up with LS_COLORS or other things
   clearenv();

   // set PATH so Intel's Math Kernel Library used by MATLAB doesn't crash
   // when loading mkl.cfg (used by POLYFIT and others)
   setenv("PATH", "/bin:/usr/bin", 0);

   printf("Trying to do an engOpen\n");
   ret = engOpen("i386 " MATLAB_EXE_PATH);
   printf("engOpen done\n");

   if (chdir(current_path) < 0) {
      if (ret) {
	 engClose(ret);
      }

      return NULL;
   }

   return ret;
}

static void populate_image_data(int img_height, int img_width, 
		const unsigned char * diamond_buf,
		unsigned char * matlab_buf)
{
   /* MATLAB arranges image data in a strange way */
   int img_row, img_col, i;
   for (img_row = 0; img_row < img_height; img_row++) {
      for (img_col = 0; img_col < img_width; img_col++) {
	 for (i = 0; i < matlab_bytes_per_pixel; i++) {
	    matlab_buf[img_row + img_col * img_height + 
		       img_height * img_width * i] =
	       diamond_buf[img_col * diamond_bytes_per_pixel +
			   img_row * diamond_bytes_per_pixel * img_width + i];
	 }
      }
   }
}

static void populate_rgbimage(Engine *eng,
			      mxArray *matrix, lf_obj_handle_t ohandle) {
  // convert the image into a normalized RGB
  engPutVariable(eng, matlab_img_name, matrix);
  engEvalString(eng, matlab_img_name " = im2uint8(" matlab_img_name ")");

  mxArray *uint8_img = engGetVariable(eng, matlab_img_name);
  if (mxGetNumberOfDimensions(uint8_img) < 3) {
    // make into RGB
    engEvalString(eng, matlab_img_name " = cat(3, " matlab_img_name ", " matlab_img_name ", " matlab_img_name ")");
    mxDestroyArray(uint8_img);
    uint8_img = engGetVariable(eng, matlab_img_name);
  }

  assert(mxGetNumberOfDimensions(uint8_img) == 3);
  assert(mxGetClassID(uint8_img) == mxUINT8_CLASS);

  const size_t *dims;
  dims = mxGetDimensions(uint8_img);
  printf("[ %d %d %d ]\n", dims[0], dims[1], dims[2]);
  assert(dims[2] == matlab_bytes_per_pixel);

  int h = dims[0];
  int w = dims[1];

  printf("writing rows: %d, cols: %d\n", h, w);
  lf_write_attr(ohandle, "_rows.int", sizeof(int), (unsigned char *) &h);
  lf_write_attr(ohandle, "_cols.int", sizeof(int), (unsigned char *) &w);

  size_t len = w * h * 4 + 16;   // 16 == RGBImage header
  unsigned char *diamond_buf = calloc(1, len);
  unsigned char *matlab_buf = (unsigned char *) mxGetData(uint8_img);

  // type is 0

  // write nbytes
  *((int *) (diamond_buf + 4)) = len;

  // write h
  *((int *) (diamond_buf + 8)) = h;

  // write w
  *((int *) (diamond_buf + 12)) = w;

  // write data
  int img_row, img_col, i;
  for (img_row = 0; img_row < h; img_row++) {
    for (img_col = 0; img_col < w; img_col++) {
      // alpha
      diamond_buf[img_col * diamond_bytes_per_pixel +
		  img_row * diamond_bytes_per_pixel * w + 3 + 16] = 255;
      // RGB
      for (i = 0; i < matlab_bytes_per_pixel; i++) {
	diamond_buf[img_col * diamond_bytes_per_pixel +
		    img_row * diamond_bytes_per_pixel * w + i + 16] =
	  matlab_buf[img_row + img_col * h + h * w * i];
      }
    }
  }

  printf("writing rgbimage of len: %d\n", len);
  lf_write_attr(ohandle, "_rgb_image.rgbimage", len, diamond_buf);

  free(diamond_buf);
  mxDestroyArray(uint8_img);
}


int f_init_matlab_exec (int num_arg, char **args, int bloblen,
                        void *blob_data, const char *filter_name,
                        void **filter_args)
{
   printf("MATLAB exec filter executing!\n");

   struct filter_instance *inst = 
      (struct filter_instance *)malloc(sizeof(struct filter_instance));

   if (!inst) {
      return -1;
   }

   if (num_arg < 2) {
      free(inst);
      printf("Invalid arguments\n");
      return -1;
   }
   
   safe_strncpy(inst->init_matlab_cmd, args[0],
		sizeof(inst->init_matlab_cmd));

   snprintf(inst->eval_matlab_cmd, sizeof(inst->eval_matlab_cmd), "[ans image] = %s(%s)",
	    args[1], matlab_img_name);

   safe_strncpy(inst->src_dir_name, "/tmp/matlab_src_XXXXXX",
		sizeof(inst->src_dir_name));

   if (mkdtemp(inst->src_dir_name) == NULL) {
      free(inst);
      printf("Could not create directory\n");
      return -1;
   }

   printf("want to untar blob of %d size\n", bloblen);
   if (untar_blob(inst->src_dir_name, bloblen, (char *)blob_data) < 0) {
      printf("Colud not untar source\n");
      destroy_src_dir(inst->src_dir_name);
      free(inst);
      return -1;
   }

   printf("Opening MATLAB engine in dir \"%s\"\n", inst->src_dir_name);

   inst->eng = open_engine_in_src_dir(inst->src_dir_name);
   if (!inst->eng) {
      printf("Could not open engine\n");
      destroy_src_dir(inst->src_dir_name);
      free(inst);
      return -1;
   }

   printf("Running init function in MATLAB\n");

   engEvalString(inst->eng, inst->init_matlab_cmd);

   // codec?
   inst->is_codec = (strcmp("RGB", filter_name) == 0);

   (*filter_args) = (void *)inst;

   printf("Filter init successful!\n");
   return 0;
}

static mxArray* create_matlab_image (lf_obj_handle_t ohandle, bool is_codec)
{
   mxArray *matlab_img;

   size_t len;
   unsigned char *diamond_attr;

   if (!is_codec) {
     /* we need a 3d matlab array of height x width x bpp */
     size_t dims[3];

     lf_ref_attr(ohandle, "_rows.int", &len, &diamond_attr);
     dims[0] = *((int *) diamond_attr);

     lf_ref_attr(ohandle, "_cols.int", &len, &diamond_attr);
     dims[1] = *((int *) diamond_attr);

     dims[2] = matlab_bytes_per_pixel;

     matlab_img = mxCreateNumericArray(3, dims, mxUINT8_CLASS, mxREAL);

     lf_ref_attr(ohandle, "_rgb_image.rgbimage", &len, &diamond_attr);
     populate_image_data(dims[0], dims[1], diamond_attr, 
			 (unsigned char *)mxGetData(matlab_img));
   } else {
     /* try to read as MAT file */
     const char *dummy;

     /* write out tempfile */
     gchar *tmp;
     int tmpfd = g_file_open_tmp(NULL, &tmp, NULL);
     FILE *tmpfile = fdopen(tmpfd, "w");
     size_t len;
     unsigned char *buf;

     lf_ref_attr(ohandle, "", &len, &buf);
     fwrite(buf, len, 1, tmpfile);
     fclose(tmpfile);

     /* mat */
     MATFile *mat = matOpen(tmp, "r");
     matlab_img = matGetNextVariable(mat, &dummy);
     matClose(mat);

     close(tmpfd);
     g_free(tmp);


   }
   return matlab_img;
}

int f_eval_matlab_exec (lf_obj_handle_t ohandle, void *filter_args)
{
   struct filter_instance *inst = (struct filter_instance *)filter_args;

   mxArray *matlab_img = create_matlab_image(ohandle, inst->is_codec);
   printf("number of mxArray elements: %d\n",
	  mxGetNumberOfElements(matlab_img));
   printf("number of mxArray dimensions: %d\n",
	  mxGetNumberOfDimensions(matlab_img));
   printf("mxArray class: %d\n",
	  mxGetClassID(matlab_img));

   engPutVariable(inst->eng, matlab_img_name, matlab_img);

   printf("going to eval \"%s\"\n", inst->eval_matlab_cmd);
   engEvalString(inst->eng, inst->eval_matlab_cmd);

   if (inst->is_codec) {
     /* also make an rgbimage for others */
     mxDestroyArray(matlab_img);
     matlab_img = engGetVariable(inst->eng, matlab_img_name);
     populate_rgbimage(inst->eng, matlab_img, ohandle);
   }

   mxArray *matlab_ret = engGetVariable(inst->eng, "ans");
   double matlab_ret_d = *(mxGetPr(matlab_ret));

   mxDestroyArray(matlab_img);
   mxDestroyArray(matlab_ret);

   lf_write_attr(ohandle, "_matlab_ans.double", sizeof(double), (unsigned char *)&matlab_ret_d);

   return (int)(matlab_ret_d);
}

int f_fini_matlab_exec (void *filter_args)
{
   struct filter_instance *inst = (struct filter_instance *)filter_args;

   engClose(inst->eng);
   destroy_src_dir(inst->src_dir_name);
   free(inst);

   return 0;
}
