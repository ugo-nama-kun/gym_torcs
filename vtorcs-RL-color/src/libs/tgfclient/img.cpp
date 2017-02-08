/***************************************************************************
                          img.cpp -- Images manipulation
                             -------------------
    created              : Tue Aug 17 20:13:08 CEST 1999
    copyright            : (C) 1999 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: img.cpp,v 1.5 2006/10/29 15:55:33 berniw Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file   
    		Images manipulation tools.
		Load and store png images with easy interface.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: img.cpp,v 1.5 2006/10/29 15:55:33 berniw Exp $
    @ingroup	img		
*/

#ifdef WIN32
#include <windows.h>
#endif

#include "png.h"

#include <tgfclient.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef WIN32
#include <direct.h>
#endif

static char buf[1024];

#define PNG_BYTES_TO_CHECK 4

/** Load an image from disk to a buffer in RGBA mode.
    @ingroup	img		
    @param	filename	name of the image to load
    @param	widthp		width of the read image
    @param	heightp		height of the read image
    @param	screen_gamma	gamma correction value
    @return	Pointer on the buffer containing the image
		<br>NULL Error
 */
unsigned char *
GfImgReadPng(const char *filename, int *widthp, int *heightp, float screen_gamma)
{
	unsigned char buf[PNG_BYTES_TO_CHECK];
	FILE *fp;
	png_structp	png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height;
	int	bit_depth, color_type, interlace_type;
	
	/*     png_color_16p	image_background; */
	double gamma;
	png_bytep *row_pointers;
	unsigned char *image_ptr, *cur_ptr;
	png_uint_32 rowbytes;
	png_uint_32 i;
	
	if ((fp = fopen(filename, "rb")) == NULL) {
		GfTrace("Can't open file %s\n", filename);
		return (unsigned char *)NULL;
	}
	
	if (fread(buf, 1, PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK) {
		GfTrace("Can't read file %s\n", filename);
		fclose(fp);
		return (unsigned char *)NULL;
	}
	
	if (png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK) != 0) {
		GfTrace("File %s not in png format\n", filename);
		fclose(fp);
		return (unsigned char *)NULL;
	}
	
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr)NULL, (png_error_ptr)NULL);
	if (png_ptr == NULL) {
		GfTrace("Img Failed to create read_struct\n");
		fclose(fp);
		return (unsigned char *)NULL;
	}
	
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
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
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
	*widthp = (int)width;
	*heightp = (int)height;
	
	if (bit_depth == 1 && color_type == PNG_COLOR_TYPE_GRAY) png_set_invert_mono(png_ptr);
	if (bit_depth == 16) {
		png_set_swap(png_ptr);
		png_set_strip_16(png_ptr);
	}
	
	if (bit_depth < 8) {
		png_set_packing(png_ptr);
	}
	
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_expand(png_ptr);
	}
	
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
		png_set_expand(png_ptr);
	}
	
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_expand(png_ptr);
	}
	
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}
	
	if (bit_depth == 8 && color_type == PNG_COLOR_TYPE_RGB) {
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	}
	
	if (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
		png_set_gamma(png_ptr, screen_gamma, gamma);
	} else {
		png_set_gamma(png_ptr, screen_gamma, 0.50);
	}
	
	png_read_update_info(png_ptr, info_ptr);
	rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	
	// RGBA expected.
	if (rowbytes != (4 * width)) {
		GfTrace("%s bad byte count... %u instead of %u\n", filename, rowbytes, 4 * width);
		fclose(fp);
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return (unsigned char *)NULL;
	}
	
	row_pointers = (png_bytep*)malloc(height * sizeof(png_bytep));
	if (row_pointers == NULL) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return (unsigned char *)NULL;
	}
	
	image_ptr = (unsigned char *)malloc(height * rowbytes);
	if (image_ptr == NULL) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return (unsigned char *)NULL;
	}
	
	for (i = 0, cur_ptr = image_ptr + (height - 1) * rowbytes ; i < height; i++, cur_ptr -= rowbytes) {
		row_pointers[i] = cur_ptr;
	}
	
	png_read_image(png_ptr, row_pointers);
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	free(row_pointers);
	
	fclose(fp);
	return image_ptr;
}


/** Write a buffer to a png image on disk.
    @ingroup	img
    @param	img		image data (RGB)
    @param	filename	filename of the png file
    @param	width		width of the image
    @param	height		height of the image
    @return	0 Ok
		<br>-1 Error
 */
int
GfImgWritePng(unsigned char *img, const char *filename, int width, int height)
{
	FILE *fp;
	png_structp	png_ptr;
	png_infop info_ptr;
	png_bytep *row_pointers;
	png_uint_32 rowbytes;
	int i;
	unsigned char *cur_ptr;
#if 0
    void		*handle;
#endif
	float		screen_gamma;
	
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		GfTrace("Can't open file %s\n", filename);
		return -1;
	}
	
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr)NULL, (png_error_ptr)NULL);
	if (png_ptr == NULL) {
		return -1;
	}
	
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return -1;
	}
	
	if (setjmp(png_jmpbuf(png_ptr))) {    
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return -1;
	}
	
	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
#if 0
    handle = GfParmReadFile(GFSCR_CONF_FILE, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    screen_gamma = (float)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_GAMMA, (char*)NULL, 2.0);
    GfParmReleaseHandle(handle);
#else
	screen_gamma = 2.0;
#endif
	png_set_gAMA(png_ptr, info_ptr, screen_gamma);
	/* png_set_bgr(png_ptr);    TO INVERT THE COLORS !!!! */
	png_write_info(png_ptr, info_ptr);
	png_write_flush(png_ptr);
	
	rowbytes = width * 3;
	row_pointers = (png_bytep*)malloc(height * sizeof(png_bytep));
	
	if (row_pointers == NULL) {
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return -1;
	}
	
	for (i = 0, cur_ptr = img + (height - 1) * rowbytes ; i < height; i++, cur_ptr -= rowbytes) {
		row_pointers[i] = cur_ptr;
	}
	
	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, (png_infop)NULL);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	free(row_pointers);
	return 0;
}

/** Free the texture
    @ingroup	img
    @param	tex	texture to free
    @return	none
*/
void
GfImgFreeTex(GLuint tex)
{
	if (tex != 0) {
		glDeleteTextures(1, &tex);
	}
}

/** Read a png image into a texture.
    @ingroup	img
    @param	filename	file name of the image
    @return	None.
 */
GLuint
GfImgReadTex(char *filename)
{
	void *handle;
	float screen_gamma;
	GLbyte *tex;
	int w, h;
	GLuint retTex;

	sprintf(buf, "%s%s", GetLocalDir(), GFSCR_CONF_FILE);
	handle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	screen_gamma = (float)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_GAMMA, (char*)NULL, 2.0);
	tex = (GLbyte*)GfImgReadPng(filename, &w, &h, screen_gamma);

	if (!tex) {
		GfParmReleaseHandle(handle);
		return 0;
	}

	glGenTextures(1, &retTex);
	glBindTexture(GL_TEXTURE_2D, retTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)(tex));

	free(tex);

	GfParmReleaseHandle(handle);
	return retTex;
}
