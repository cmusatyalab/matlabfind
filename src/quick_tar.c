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

#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

#include "quick_tar.h"

static inline int min(int a, int b)
{
  return ((a < b) ? a : b);
}

int untar_blob(const char *src_dir_name, unsigned int bloblen, char *blob_data)
{
  while (bloblen > 0) {
    uint32_t name_size, file_size;
    char file_name[PATH_MAX], file_path[PATH_MAX];

    if (bloblen < 8) {
      return -1;
    }

    name_size = ntohl(*((uint32_t *)blob_data));
    blob_data += sizeof(uint32_t);
    file_size = ntohl(*((uint32_t *)blob_data));
    blob_data += sizeof(uint32_t);
    bloblen -= 2*sizeof(uint32_t);

    if (bloblen < (name_size + file_size)) {
      return -1;
    }

    safe_strncpy(file_name, blob_data, sizeof(file_name));
    blob_data += name_size;
    bloblen -= name_size;

    snprintf(file_path, sizeof(file_path), "%s/%s", src_dir_name, file_name);
    int fd = creat(file_path, 0600);
    if (fd < 0) {
      return -1;
    }

    if (write(fd, blob_data, file_size) < 0) {
      close(fd);
      return -1;
    }
    blob_data += file_size;
    bloblen -= file_size;

    close(fd);
  }

  return 0;
}

int tar_blob(const char *src_dir_name, int out_fd)
{
  int bytes_written = 0;

  DIR *src_dir = opendir(src_dir_name);
  if (!src_dir) {
    return -1;
  }

  struct dirent *src_dir_ent;
  while ( (src_dir_ent = readdir(src_dir)) ) {
    if (src_dir_ent->d_type == DT_REG || src_dir_ent->d_type == DT_UNKNOWN) {
      char file_path[PATH_MAX];
      snprintf(file_path, sizeof(file_path), "%s/%s",
	       src_dir_name, src_dir_ent->d_name);

      int in_fd = open(file_path, O_RDONLY);
      if (in_fd < 0) {
	return -1;
      }

      uint32_t file_size, name_size, file_size_buf, name_size_buf;

      name_size = strlen(src_dir_ent->d_name) + 1; /* account for \0 at the end */

      struct stat stat_buf;
      if (fstat(in_fd, &stat_buf) < 0) {
	close(in_fd);
	return -1;
      }

      // if DT_UNKNOWN above, check here
      if (!S_ISREG(stat_buf.st_mode)) {
	close(in_fd);
	return -1;
      }

      file_size = stat_buf.st_size;

      /* fix for network ordering */
      file_size_buf = htonl(file_size);
      name_size_buf = htonl(name_size);

      if (write(out_fd, &name_size_buf, sizeof(name_size_buf)) != sizeof(name_size_buf)) {
	close(in_fd);
	return -1;
      }

      if (write(out_fd, &file_size_buf, sizeof(file_size_buf)) != sizeof(file_size_buf)) {
	close(in_fd);
	return -1;
      }

      if (write(out_fd, src_dir_ent->d_name, name_size) != name_size) {
	close(in_fd);
	return -1;
      }

      char file_buf[4096];
      int bytes_remaining = file_size;
      while (bytes_remaining > 0) {
	int amt = min(bytes_remaining, sizeof(file_buf));

	if (read(in_fd, file_buf, amt) != amt) {
	  close(in_fd);
	  return -1;
	}

	if (write(out_fd, file_buf, amt) != amt) {
	  close(in_fd);
	  return -1;
	}

	bytes_remaining -= amt;
      }

      bytes_written += sizeof(name_size) + sizeof(file_size) + name_size + file_size;
      close(in_fd);
    }
  }

  return bytes_written;
}
