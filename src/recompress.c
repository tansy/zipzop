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
#include "zopfli.h"
#include "deflate.h"
#include "zopfli/src/zopfli/deflate.h"

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

///static void zopfli_deflate(const uchar *src, size_t src_size, uchar **dst_p, size_t *dst_size_p, int num_iterations)
///int64_t zlib_turbo_compress(char *inbuf, size_t insize, char *outbuf, size_t outsize, codec_options_t *codec_options)
size_t zlib_turbo_compress(char *inbuf, size_t insize, char *outbuf, size_t outsize, int clevel)
{
    z_stream strm;
    memset(&strm, 0, sizeof(strm));
    ///if (deflateInit(&strm, codec_options->level) != Z_OK)
    if (deflateInit(&strm, clevel) != Z_OK)
        return 0;
    // levels 4-9 search on worker threads, with the output of one thread; without
    // threads in the build this fails, and the stream stays on one thread
    ///if (codec_options->threads > 1)
    ///    deflateSetThreads(&strm, codec_options->threads);

    // as compress2(), which cannot take deflateSetThreads()
    const uInt max = (uInt)-1;
    size_t in_left = insize, out_left = outsize;
    strm.next_in = (Bytef*)inbuf;
    strm.next_out = (Bytef*)outbuf;
    int err;
    do {
        if (strm.avail_out == 0) {
            strm.avail_out = out_left > max ? max : (uInt)out_left;
            out_left -= strm.avail_out;
        }
        if (strm.avail_in == 0) {
            strm.avail_in = in_left > max ? max : (uInt)in_left;
            in_left -= strm.avail_in;
        }
        err = deflate(&strm, in_left ? Z_NO_FLUSH : Z_FINISH);
    } while (err == Z_OK);
    ///int64_t complen = (char*)strm.next_out - outbuf;
    size_t complen = (char*)strm.next_out - outbuf;
    deflateEnd(&strm);
    return err == Z_STREAM_END ? complen : 0;
}

#define FILENAME_BUF_SIZE 48

void recompress_entry(FILE *infile,
		      FILE *outfile,
		      LocalFileHeader *header,
		      int num_iterations) {
  char filename[FILENAME_BUF_SIZE + 1];

  copy_filename_suffix(header->ext, header->filename_len, filename, FILENAME_BUF_SIZE);
  filename[FILENAME_BUF_SIZE] = '\0';

  // In the real world, many zip files defer writing the file size until after
  // the compressed data.  This is signalled by a flag bit
  if (header->bit_flag & 8)
  {
    // Run an inflate pass to find the size of the data
    long startpos = ftell(infile);
    long insize = 0;
    long outsize = 0;
    FILE *source = infile;

    int ret;
    unsigned have;
  #define CHUNK 10000
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    z_stream strm = { .zalloc = Z_NULL, .zfree = Z_NULL, .opaque = Z_NULL };

 //   printf("File deferred writing size till after the compressed data\n");

    if (inflateInit2(&strm, -15) != Z_OK) {
      puts("ERROR: Cannot initialize inflate z_stream");
      exit(1);
    }

    // decompress until deflate stream ends or end of file
    do
    {
      strm.avail_in = fread(in, 1, CHUNK, source);
      if (ferror(source)) {
        (void)inflateEnd(&strm);
  //                   return Z_ERRNO;
      }
      if (strm.avail_in == 0)
        break;
      strm.next_in = in;

      insize += strm.avail_in;

      // run inflate() on input until output buffer not full
      do {
        strm.avail_out = CHUNK;
        strm.next_out = out;

        ret = inflate(&strm, Z_NO_FLUSH);
  //                   assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
  //                   switch (ret) {
  //                   case Z_NEED_DICT:
  //                       ret = Z_DATA_ERROR;     /* and fall through */
  //                   case Z_DATA_ERROR:
  //                   case Z_MEM_ERROR:
  //                       (void)inflateEnd(&strm);
  //                       return ret;
  //                   }

        have = CHUNK - strm.avail_out;

        outsize += have;

      } while (strm.avail_out == 0);

      // done when inflate() says it's done
    } while (ret != Z_STREAM_END);

 //   printf("Got file size compressed %ld uncompressed %ld @%ld\n", insize - strm.avail_in, outsize, startpos);
    header->comp_size = insize - strm.avail_in;
    header->uncomp_size = outsize;
    fseek(infile, startpos, SEEK_SET);
  }

  size_t src_size = header->comp_size;
  uchar *src = (uchar *)allocate_or_exit(src_size);
  read_bytes(src, src_size, infile);
  
  if (header->method != METHOD_DEFLATE) {
    write_local_file_header(outfile, header);
    write_bytes(src, src_size, outfile);
    free(src);
    return;
  }

  printf("%-*s : %zd -> ", FILENAME_BUF_SIZE, filename, src_size);

  size_t uncomp_size = header->uncomp_size;
  uchar *tmp = (uchar *)allocate_or_exit(uncomp_size);
  inflate_src(src, src_size, tmp, uncomp_size);

  uchar *dst = NULL;
  size_t dst_size = 0;
  if (num_iterations <= 12) {
    dst_size = (size_t)uncomp_size*1.0000825+30;
    dst = (uchar *)allocate_or_exit(dst_size);
    ///size_t zlib_turbo_compress(char *inbuf, size_t insize, char *outbuf, size_t outsize, int clevel)
    dst_size = zlib_turbo_compress((char*)tmp, uncomp_size, (char*)dst, dst_size, num_iterations);
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
  printf("%zd bytes (%zd bytes = %zd%%) \n", dst_size, dst_size - src_size, 100 * (src_size - dst_size) / src_size );

  write_local_file_header(outfile, header);
  write_bytes(dst, dst_size, outfile);

  free(dst);
  free(src);

  // Now we need to step over any deferred size data that is in the stream
  if (header->bit_flag & 8)
  {
    // Deferred data is present
    int first =  read_u32(infile);
    if (first == 0x08074b50)
    {
      // It includes a signature, so a larger step over is needed
      fseek(infile, 12, SEEK_CUR);
    }
    else
    {
      // Just step over the smaller structure
      fseek(infile, 8, SEEK_CUR);
    }
  }
}
