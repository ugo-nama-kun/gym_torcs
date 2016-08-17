/***************************************************************************

    file        : png2jpg.cpp
    created     : Thu Dec  4 20:16:09 CET 2003
    copyright   : (C) 2003 by Eric Espié                        
    email       : eric.espie@torcs.org   
    version     : $Id: png2jpg.c,v 1.1 2003/12/06 16:36:24 torcs Exp $                                  

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file   PNG to JPEG converter
    		
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id: png2jpg.c,v 1.1 2003/12/06 16:36:24 torcs Exp $
*/

#ifdef WIN32
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#ifdef WIN32
#include <direct.h>
#endif

#include "png.h"
#include "jpeglib.h"

static char buf[1024];

#define PNG_BYTES_TO_CHECK 4

/** Load an image from disk to a buffer in RGBA mode.
    @ingroup	img		
    @param	widthp		width of the read image
    @param	heightp		height of the read image
    @param	screen_gamma	gamma correction value
    @return	Pointer on the buffer containing the image
		<br>NULL Error
 */
unsigned char *
ImgReadPng(FILE *fp, int *widthp, int *heightp, float screen_gamma)
{
    unsigned char 	buf[PNG_BYTES_TO_CHECK];
    png_structp		png_ptr;
    png_infop		info_ptr;
    png_uint_32		width, height;
    int			bit_depth, color_type, interlace_type;
/*     png_color_16p	image_background; */
    double		gamma;
    png_bytep		*row_pointers;
    unsigned char	*image_ptr, *cur_ptr;
    png_uint_32		rowbytes;
    png_uint_32		i;
    
    if (fread(buf, 1, PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK) {
	fprintf(stderr, "Can't read file\n");
	fclose(fp);
	return (unsigned char *)NULL;
    }

    if (png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK) != 0) {
	fprintf(stderr, "File not in png format\n");
	fclose(fp);
	return (unsigned char *)NULL;
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr)NULL, (png_error_ptr)NULL);
    if (png_ptr == NULL) {
	fprintf(stderr, "Img Failed to create read_struct\n");
	fclose(fp);
	return (unsigned char *)NULL;
    }
   
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
	{
	    fclose(fp);
	    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
	    return (unsigned char *)NULL;
	}
   
    if (setjmp(png_jmpbuf(png_ptr)))
	{
	    /* Free all of the memory associated with the png_ptr and info_ptr */
	    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	    fclose(fp);
	    /* If we get here, we had a problem reading the file */
	    return (unsigned char *)NULL;
	}

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);
/*     png_set_invert_alpha(png_ptr); */
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
		 &interlace_type, NULL, NULL);
    *widthp = (int)width;
    *heightp = (int)height;

    if (bit_depth == 1 && color_type == PNG_COLOR_TYPE_GRAY) png_set_invert_mono(png_ptr);
    if (bit_depth == 16) {
	png_set_swap(png_ptr);
	png_set_strip_16(png_ptr);
    }
    if (bit_depth < 8) png_set_packing(png_ptr);
    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_expand(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_expand(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);
/*     if (png_get_bKGD(png_ptr, info_ptr, &image_background)) */
/*         png_set_background(png_ptr, image_background, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0); */
    if (bit_depth == 8 && color_type == PNG_COLOR_TYPE_RGB) png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
    if (png_get_gAMA(png_ptr, info_ptr, &gamma))
	png_set_gamma(png_ptr, screen_gamma, gamma);
    else
	png_set_gamma(png_ptr, screen_gamma, 0.50);
    png_read_update_info(png_ptr, info_ptr);
    rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    /* RGB expected... */
    if (rowbytes != (4 * width)) {
	fprintf(stderr, "input: bad byte count... %ld instead of %ld\n", rowbytes, 4 * width);
	fclose(fp);
	png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
	return (unsigned char *)NULL;
    }       
    row_pointers = (png_bytep*)malloc(height * sizeof(png_bytep));
    if (row_pointers == NULL)
	{
	    fclose(fp);
	    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
	    return (unsigned char *)NULL;
	}
    image_ptr = (unsigned char *)malloc(height * rowbytes);
    if (image_ptr == NULL)
	{
	    fclose(fp);
	    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
	    return (unsigned char *)NULL;
	}
    for (i = 0, cur_ptr = image_ptr ; i < height; i++, cur_ptr += rowbytes) {
	row_pointers[i] = cur_ptr;
    }
    png_read_image(png_ptr, row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    free(row_pointers);

    fclose(fp);
    return image_ptr;
}

/*
 * Sample routine for JPEG compression.  We assume that the target file name
 * and a compression quality factor are passed in.
 */

void
write_JPEG_file (FILE *outfile, unsigned char *image_buffer, int image_width, int image_height, int quality)
{
  /* This struct contains the JPEG compression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   * It is possible to have several such structures, representing multiple
   * compression/decompression processes, in existence at once.  We refer
   * to any one struct (and its associated working data) as a "JPEG object".
   */
  struct jpeg_compress_struct cinfo;
  /* This struct represents a JPEG error handler.  It is declared separately
   * because applications often want to supply a specialized error handler
   * (see the second half of this file for an example).  But here we just
   * take the easy way out and use the standard error handler, which will
   * print a message on stderr and call exit() if compression fails.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct jpeg_error_mgr jerr;
  /* More stuff */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  /* Step 1: allocate and initialize JPEG compression object */

  /* We have to set up the error handler first, in case the initialization
   * step fails.  (Unlikely, but it could happen if you are out of memory.)
   * This routine fills in the contents of struct jerr, and returns jerr's
   * address which we place into the link field in cinfo.
   */
  cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  /* Step 2: specify data destination (eg, a file) */
  /* Note: steps 2 and 3 can be done in either order. */

  jpeg_stdio_dest(&cinfo, outfile);

  /* Step 3: set parameters for compression */

  /* First we supply a description of the input image.
   * Four fields of the cinfo struct must be filled in:
   */
  cinfo.image_width = image_width; 	/* image width and height, in pixels */
  cinfo.image_height = image_height;
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  /* Now use the library's routine to set default compression parameters.
   * (You must set at least cinfo.in_color_space before calling this,
   * since the defaults depend on the source color space.)
   */
  jpeg_set_defaults(&cinfo);
  /* Now you can set any non-default parameters you wish to.
   * Here we just illustrate the use of quality (quantization table) scaling:
   */
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  /* Step 4: Start compressor */

  /* TRUE ensures that we will write a complete interchange-JPEG file.
   * Pass TRUE unless you are very sure of what you're doing.
   */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  /* Here we use the library's state variable cinfo.next_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   * To keep things simple, we pass one scanline per call; you can pass
   * more if you wish, though.
   */
  row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  /* Step 6: Finish compression */

  jpeg_finish_compress(&cinfo);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_compress(&cinfo);

  /* And we're done! */
}

static char 	*infile;
static char 	*outfile;
static int	quality;
static float	gamma;

static void usage(void)
{
    fprintf(stderr, "PNG to JPEG image converter $Revision: 1.1 $ \n");
    fprintf(stderr, "Usage: png2jpg [-i input.png] [-o output.jpg] [-q quality] [-g gamma]\n");
    fprintf(stderr, "       -i input.png  : input PNG file (default stdin)\n");
    fprintf(stderr, "       -o output.jpg : output JPEG file (default input.jpg)\n");
    fprintf(stderr, "       -q quality    : JPEG file quality [0..100] (default 90)\n");
    fprintf(stderr, "       -g gamma      : PNG gamma value (default 0.5)\n");
}

static void init_args(int argc, char **argv)
{
    int	i;
    
    infile = NULL;
    outfile = NULL;
    quality = 90;
    gamma = 0.5;
    
    i = 1;
    while (i < argc) {
	if ((strncmp(argv[i], "-h", 2) == 0)
	    || (strncmp(argv[i], "-?", 2) == 0)
	    || (strncmp(argv[i], "--help", 6) == 0)) {
	    usage();
	    exit(0);
	}

	if (strncmp(argv[i], "-v", 2) == 0) {
	    printf("PNG to JPEG image converter $Revision: 1.1 $ \n");
	    exit(0);
	}

	if (strncmp(argv[i], "-i", 2) == 0) {
	    if (i + 1 < argc) {
		infile = strdup(argv[++i]);
	    } else {
		usage();
		exit(0);
	    }
	} else if (strncmp(argv[i], "-o", 2) == 0) {
	    if (i + 1 < argc) {
		outfile = strdup(argv[++i]);
	    } else {
		usage();
		exit(0);
	    }
	} else if (strncmp(argv[i], "-q", 2) == 0) {
	    if (i + 1 < argc) {
		quality = strtol(argv[++i], NULL, 0);
	    } else {
		usage();
		exit(0);
	    }
	} else if (strncmp(argv[i], "-g", 2) == 0) {
	    if (i + 1 < argc) {
		sscanf(argv[++i], "%g", &gamma);
	    } else {
		usage();
		exit(0);
	    }
	} else {
	    usage();
	    exit(0);
	}
	i++;
    }
}


int main (int argc, char **argv)
{
    FILE		*fin;
    FILE		*fout;
    char		*s;
    unsigned char	*img1, *ip1;
    unsigned char	*img2, *ip2;    
    int			width, height;
    int			i, len;

    init_args (argc, argv);
    
    if (infile) {
	fin = fopen (infile, "rb");
	if (!fin) {
	    perror (infile);
	    exit (1);
	}
    } else {
	fin = stdin;
    }
    
    if (outfile) {
	if ((fout = fopen(outfile, "wb")) == NULL) {
	    perror (outfile);
	    exit(1);
	}
    } else {
	if (infile) {
	    outfile = strdup(infile);
	    s = outfile + strlen(outfile) - 3;
	    sprintf(s, "jpg");
	    if ((fout = fopen(outfile, "wb")) == NULL) {
		perror (outfile);
		exit(1);
	    }
	} else {
	    fout = stdout;
	}
    }
    
    img1 = ImgReadPng(fin, &width, &height, gamma);
    if (!img1) {
	fprintf(stderr, "Error while reading the file\n");
	exit(1);
    }
    
    /* Strip the alpha channel
     * and revert the lines 
     */
    img2 = (unsigned char *)malloc(width * height * 3);
    ip1 = img1;
    ip2 = img2;
    len = width * height * 4;    
    for (i = 1; i <= len; i++) {
	if (i % 4) {
	    *ip2 = *ip1;
	    ip2++;
	}
	ip1++;
    }
    
    write_JPEG_file (fout, img2, width, height, quality);
    
    return 0;
}

    
