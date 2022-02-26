/*
 * Copyright (c) 2013 KOMIYA Atsushi
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

#include <string.h>

#include <zlib.h>
#include "zipzop.h"
#include "deflate.h"
#include "libdeflate.h"
#include <assert.h>

const int METHOD_DEFLATE = 8;

static void inflate_src(uchar *src, size_t src_size,
			uchar *dst, size_t dst_size) {
  z_stream zs = { .zalloc = Z_NULL, .zfree = Z_NULL, .opaque = Z_NULL };
  if (inflateInit2(&zs, -15) != Z_OK) {
    puts("ERROR: Cannot initialize inflate z_stream");
    exit(1);
  }

  zs.next_in = src;
  zs.avail_in = src_size;
  zs.next_out = dst;
  zs.avail_out = dst_size;

  while (zs.avail_out > 0 && zs.avail_in > 0) {
    int ret = inflate(&zs, Z_NO_FLUSH);
    if (ret == Z_STREAM_END) {
      return;
    }

    if (ret != Z_OK) {
      printf("ERROR: Compressed file is corrupted (inflate() returns %d)\n", ret);
      exit(1);
    }
  }
}

static void zopfli_deflate(const uchar *src, size_t src_size,
			   uchar **dst_p, size_t *dst_size_p,
			   int num_iterations) {
  ZopfliOptions options;
  ZopfliInitOptions(&options);
  options.numiterations = num_iterations;

  uchar b = 0;
  ZopfliDeflate(&options, 2, 1,
	  src, src_size,
	  &b,
	  dst_p, dst_size_p);
}

static void libdeflate_deflate(const uchar *src, size_t src_size,
			   uchar **dst_p, size_t *dst_size_p,
			   int level) {
  static struct libdeflate_compressor *ld_compressor;
  ld_compressor = libdeflate_alloc_compressor(level<12 ? level : 12);
  if (ld_compressor == NULL)
    fprintf(stderr, "error: libdeflate_alloc_compressor(): NULL\n");
  assert(ld_compressor != NULL);
  //libdeflate_deflate_compress(struct libdeflate_compressor *compressor,
  //                            const void *in, size_t in_nbytes,
  //                            void *out, size_t out_nbytes_avail);
  *dst_size_p = libdeflate_deflate_compress(ld_compressor, src, src_size, *dst_p, *dst_size_p);
  if (*dst_size_p == 0)
    fprintf(stderr, "error: libdeflate_zlib_compress()\n");
}

static void copy_filename_suffix(const char *src, size_t src_size,
				 char *buf, size_t buf_size) {
  if (src_size <= buf_size) {
    strncpy(buf, src, src_size);

    if (src_size < buf_size) {
      buf[src_size] = '\0';
    }

    return;
  }

  buf[0] = buf[1] = buf[2] = '.';
  size_t copy_len = buf_size - 3;
  strncpy(buf + 3, src + src_size - copy_len, copy_len);
}

#define FILENAME_BUF_SIZE 32

void recompress_entry(FILE *infile,
		      FILE *outfile,
		      LocalFileHeader *header,
		      int num_iterations) {
  char filename[FILENAME_BUF_SIZE + 1];
  copy_filename_suffix(header->ext, header->filename_len, filename, FILENAME_BUF_SIZE);
  filename[FILENAME_BUF_SIZE] = '\0';

  size_t src_size = header->comp_size; // compressed input deflate stream size
  uchar *src = (uchar *)allocate_or_exit(src_size); // compressed input deflate stream
  read_bytes(src, src_size, infile);

  if (header->method != METHOD_DEFLATE) {
    write_local_file_header(outfile, header);
    write_bytes(src, src_size, outfile);
    free(src);
    return;
  }

  printf("%-32s : %zd -> ", filename, src_size);

  size_t uncomp_size = header->uncomp_size; // uncompressed stream size
  uchar *tmp = (uchar *)allocate_or_exit(uncomp_size); // uncompressed stream
  inflate_src(src, src_size, tmp, uncomp_size);

  uchar *dst = NULL;
  size_t dst_size = 0;
  if (num_iterations <= 12) {
    dst_size = (size_t)uncomp_size*1.05+5000+8+1;
    dst = (uchar *)allocate_or_exit(dst_size);
    libdeflate_deflate(tmp, uncomp_size, &dst, &dst_size, num_iterations);
    ///fprintf(stderr, "lv=%02d: src_size=%d, uncomp_size=%d, dst_size=%d\n", num_iterations, src_size, uncomp_size, dst_size);
  }
  else {
    zopfli_deflate(tmp, uncomp_size, &dst, &dst_size, num_iterations);
  }

  if (dst_size > src_size) {
    free(dst);
    dst = src;
    dst_size = src_size;
    src = NULL;
  }

  header->comp_size = dst_size;
  printf("%zd bytes (%zd bytes)\n", dst_size, dst_size - src_size);

  write_local_file_header(outfile, header);
  write_bytes(dst, dst_size, outfile);

  free(dst);
  free(src);
}
