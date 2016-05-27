/***************************************************************************

    file                 : grtrackmap.cpp
    created              : Fri Aug 29 00:58:00 CEST 2003
    copyright            : (C) 2003 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: grtrackmap.cpp,v 1.11 2005/06/03 23:51:20 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
	This class displays an overview map of the track, such that you can see the
	upcoming track layout. Your car is displayed as a dot.
	To get reasonable efficiency, the track is first rendered into a texture. During the
	game just the texture needs to be redrawn.
*/

#include "grtrackmap.h"
#include <tgfclient.h>

// The resolution in [m] to analyse turns.
const float cGrTrackMap::RESOLUTION = 5.0;

// Minimum and Maximum line width in pixels to draw the track.
const float cGrTrackMap::MINLINEWIDTH = 5.0;
const float cGrTrackMap::MAXLINEWIDTH = 20.0;

// Some data needs just one initalization, after first initialization this is set to true.
bool cGrTrackMap::isinitalized = false;

// Track overview texture object.
GLuint cGrTrackMap::mapTexture;

// The car "dot" display list.
GLuint cGrTrackMap::cardot;

// Track bounding box properties, lower left, upper right corner, width and height.
float cGrTrackMap::track_min_x;
float cGrTrackMap::track_max_x;
float cGrTrackMap::track_min_y;
float cGrTrackMap::track_max_y;
float cGrTrackMap::track_width;
float cGrTrackMap::track_height;

// The ratio of width and height to MAX(width, height)
float cGrTrackMap::track_x_ratio;
float cGrTrackMap::track_y_ratio;

// Position and size of the map (relative to top left).
int cGrTrackMap::map_x;
int cGrTrackMap::map_y;
int cGrTrackMap::map_size;

// Scaling factor from meters to texels.
float cGrTrackMap::ratio;

// Color of the cars "dots".
GLfloat cGrTrackMap::currentCarColor[4];
GLfloat cGrTrackMap::aheadCarColor[4];
GLfloat cGrTrackMap::behindCarColor[4];


// The constructor creates a texture of the track data, such that the track
// layout can be displayed efficiently. Additional data gets initialized.
cGrTrackMap::cGrTrackMap()
{
	// Hardcoded initial view mode, available per instance.
	viewmode = TRACK_MAP_NORMAL_WITH_OPPONENTS;

	// For all views we need the same track texture, so create it just once.
	if (isinitalized) {
		return;
	} else {
		// Initialize colors for the various car "dots".
		initColors();

		tTrack *track = grTrack;
		tTrackSeg* first = track->seg;
		tTrackSeg* seg = first;

		// Search the maximum/minimum x/y values of the track (axis aligned bounding box),
		// to compute later the texture parameters and to be able to place the cars (dots)
		// correct. The impementation is inefficient, but it's just executed one time per
		// race, so it doesn't matter.
		track_min_x = FLT_MAX;
		track_max_x = -FLT_MAX;
		track_min_y = FLT_MAX;
		track_max_y = -FLT_MAX;

		do {
			// We analyse the straight and turns different, because (read on)
			if (seg->type == TR_STR) {
				// Straights are trivial, because the corners are sufficient to create a
				// bounding box.
				// TODO: If CCW/CW is known, you just need to check one side (the outside).
				// TODO: More effiecient checking.
				checkAndSetMinimum(track_min_x, seg->vertex[TR_SL].x);
				checkAndSetMinimum(track_min_x, seg->vertex[TR_SR].x);
				checkAndSetMinimum(track_min_y, seg->vertex[TR_SL].y);
				checkAndSetMinimum(track_min_y, seg->vertex[TR_SR].y);

				checkAndSetMaximum(track_max_x, seg->vertex[TR_SL].x);
				checkAndSetMaximum(track_max_x, seg->vertex[TR_SR].x);
				checkAndSetMaximum(track_max_y, seg->vertex[TR_SL].y);
				checkAndSetMaximum(track_max_y, seg->vertex[TR_SR].y);
			} else {
				// Turns are not that easy, think of a definition of a circle or an arc. If you
				// just consider the corners the bounding box might be any order too small
				// or too big.
				// To avoid that we create intermediate steps, such that we get a defined
				// accuracy (e.g. 5 meters).
				float curseglen = 0.0;
				float dphi = RESOLUTION / seg->radius;
				double xc = seg->center.x;
				double yc = seg->center.y;
				dphi = (seg->type == TR_LFT) ? dphi : -dphi;
				float phi = 0.0;

				while (curseglen < seg->length) {
					float cs = cos(phi), ss = sin(phi);
					// TODO: If CCW/CW is known, you just need to check one side (the outside).
					float lx, ly, rx, ry;
					lx = seg->vertex[TR_SL].x * cs - seg->vertex[TR_SL].y * ss - xc * cs + yc * ss + xc;
					ly = seg->vertex[TR_SL].x * ss + seg->vertex[TR_SL].y * cs - xc * ss - yc * cs + yc;

					rx = seg->vertex[TR_SR].x * cs - seg->vertex[TR_SR].y * ss - xc * cs + yc * ss + xc;
					ry = seg->vertex[TR_SR].x * ss + seg->vertex[TR_SR].y * cs - xc * ss - yc * cs + yc;

					// TODO: More effiecient checking.
					checkAndSetMinimum(track_min_x, lx);
					checkAndSetMinimum(track_min_x, rx);
					checkAndSetMinimum(track_min_y, ly);
					checkAndSetMinimum(track_min_y, ry);

					checkAndSetMaximum(track_max_x, lx);
					checkAndSetMaximum(track_max_x, rx);
					checkAndSetMaximum(track_max_y, ly);
					checkAndSetMaximum(track_max_y, ry);

					curseglen += RESOLUTION;
					phi += dphi;
				}
			}
			seg = seg->next;
		} while (seg != first);

		// Compute the maximum possible texture size possible to create the track texture.
		// TODO: use pbuffer if available or subdivide and render/readback in multiple passes.
		int texturesize = 1;
		int maxtexturesize = MIN(grWinw, grWinh);
		while (texturesize <= maxtexturesize) {
			texturesize <<= 1;
		}
		texturesize >>= 1;

		// Get maximum OpenGL texture size and reduce texturesize if necessary.
		int maxOpenGLtexturesize;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxOpenGLtexturesize);
		if (texturesize > maxOpenGLtexturesize) {
			texturesize = maxOpenGLtexturesize;
		}

		// Compute an estimate of the overall width and height of the track in [m].
		track_width = track_max_x - track_min_x;
		track_height = track_max_y - track_min_y;

		// Compute the final line width to draw the track.
		float linewidth = MIN(MAXLINEWIDTH, MINLINEWIDTH*4000.0/MAX(track_width, track_height));
		linewidth = linewidth*(texturesize/512.0);

		// Compute a first estimate of the pixel to distance ratio.
		ratio = texturesize/MAX(track_width, track_height);

		// Compute final minimum/maximum values.
		track_max_x = track_max_x + RESOLUTION + linewidth/ratio;
		track_max_y = track_max_y + RESOLUTION + linewidth/ratio;
		track_min_x = track_min_x - RESOLUTION - linewidth/ratio;
		track_min_y = track_min_y - RESOLUTION - linewidth/ratio;

		// Compute final ratio, width and height.
		track_width = track_max_x - track_min_x;
		track_height = track_max_y - track_min_y;
		ratio = texturesize/MAX(track_width, track_height);

		// Compute the ratios
		if (track_width >= track_height) {
			track_x_ratio = 1.0;
			track_y_ratio = track_height/track_width;
		} else {
			track_y_ratio = 1.0;
			track_x_ratio = track_width/track_height;
		}

		isinitalized = true;

		// Now we are ready to render the track into the framebuffer (backbuffer).
		// Clear the framebuffer (backbuffer), make it "transparent" (alpha = 0.0).
		glFinish();
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);

		// Set transformations and projection such that one pixel is one unit.
		glViewport (grWinx, grWiny, grWinw, grWinh);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0.0, grWinw, 0.0, grWinh);
		glMatrixMode(GL_MODELVIEW);

		// Now draw the track as quad strip. The reason for that is that drawing with
		// glEnable(GL_LINE_SMOOTH) and glHint(GL_LINE_SMOOTH_HINT, GL_NICEST) caused problems
		// with the alpha channel.
		float halflinewidth = linewidth/2.0;
		bool firstvert = true;
		float xf1, yf1 , xf2, yf2;
		xf1 = yf1 = xf2 = yf2 = 0.0;
		glBegin(GL_QUAD_STRIP);
		seg = first;
		do {
			if (seg->type == TR_STR) {
				// Draw a straight.
				// Compute global coordinate on track middle.
				float xm = (seg->vertex[TR_SL].x + seg->vertex[TR_SR].x) / 2.0;
				float ym = (seg->vertex[TR_SL].y + seg->vertex[TR_SR].y) / 2.0;
				xm = (xm - track_min_x)*ratio;
				ym = (ym - track_min_y)*ratio;

				// Compute normal to the track middle.
				float xn = seg->vertex[TR_SL].x - seg->vertex[TR_SR].x;
				float yn = seg->vertex[TR_SL].y - seg->vertex[TR_SR].y;
				float length = sqrt(xn*xn+yn*yn);
				xn /= length;
				yn /= length;

				// Compute the new points.
				float x1 = (xm - xn*halflinewidth);
				float y1 = (ym - yn*halflinewidth);
				float x2 = (xm + xn*halflinewidth);
				float y2 = (ym + yn*halflinewidth);

				if (firstvert) {
					firstvert = false;
					xf1 = x1;
					yf1 = y1;
					xf2 = x2;
					yf2 = y2;
				}
				glVertex2f(x1, y1);
				glVertex2f(x2, y2);
			} else {
				// To draw the turns correct we subdivide them again, like above.
				float curseglen = 0.0;
				float dphi = RESOLUTION / seg->radius;
				double xc = seg->center.x;
				double yc = seg->center.y;
				dphi = (seg->type == TR_LFT) ? dphi : -dphi;
				float mx = (seg->vertex[TR_SL].x + seg->vertex[TR_SR].x) / 2.0;
				float my = (seg->vertex[TR_SL].y + seg->vertex[TR_SR].y) / 2.0;
			    float phi = 0.0;

				while (curseglen < seg->length) {
					float cs = cos(phi), ss = sin(phi);
					float x = mx * cs - my * ss - xc * cs + yc * ss + xc;
					float y = mx * ss + my * cs - xc * ss - yc * cs + yc;

					float xn = x - xc;
					float yn = y - yc;
					float length = sqrt(xn*xn+yn*yn);
					xn /= length;
					yn /= length;

					x = (x - track_min_x)*ratio;
					y = (y - track_min_y)*ratio;

					float x1 = (x + xn*halflinewidth);
					float y1 = (y + yn*halflinewidth);
					float x2 = (x - xn*halflinewidth);
					float y2 = (y - yn*halflinewidth);

					if (seg->type == TR_LFT) {
						glVertex2f(x1, y1);
						glVertex2f(x2, y2);
						if (firstvert) {
							firstvert = false;
							xf1 = x1;
							yf1 = y1;
							xf2 = x2;
							yf2 = y2;
						}
					} else {
						glVertex2f(x2, y2);
						glVertex2f(x1, y1);
						if (firstvert) {
							firstvert = false;
							xf1 = x2;
							yf1 = y2;
							xf2 = x1;
							yf2 = y1;
						}
					}

					curseglen += RESOLUTION;
					phi += dphi;
				}
			}
			seg = seg->next;
		} while (seg != first);

		if (!firstvert) {
			glVertex2f(xf1, yf1);
			glVertex2f(xf2, yf2);
		}
		glEnd();

		// Read track picture into memory to be able to generate mipmaps. I read back an RGBA
		// image because I don't know yet what people want to add to the map, so RGBA is most
		// flexible to add things like start line, elevation coloring etc.
		// Do not use GL_ALPHA or GL_LUMINANCE to save memory, somehow this leads to a
		// performance penalty (at least on NVidia cards).
		GLuint *trackImage = (GLuint*) malloc(texturesize*texturesize*sizeof(GLuint));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadBuffer(GL_BACK);
		glReadPixels(0, 0, texturesize, texturesize, GL_RGBA, GL_BYTE, trackImage);

		// Check if the color buffer has alpha, if not fix the texture. Black gets
		// replaced by transparent black, so don't draw black in the texture, you
		// won't see anything.
		if (glutGet(GLUT_WINDOW_ALPHA_SIZE) == 0) {
			// There is no alpha, so we fix it manually. Because we added a little border
			// around the track map the first pixel should always contain the
			// clearcolor.
			GLuint clearcolor = trackImage[0];
			int i;
			for (i = 0; i < texturesize*texturesize; i++) {
				if (trackImage[i] == clearcolor) {
					// Assumption: transparent black is 0, portable?
					trackImage[i] = 0;
				}
			}
		}

		// Finally copy the created image of the track from the framebuffer into the texture.
		glGenTextures (1, &mapTexture);
		glBindTexture(GL_TEXTURE_2D, mapTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);


		// If GL_ARB_texture_compression is available at runtime, (try to) compress the
		// texture. This is done with the specification of the internal format to
		// GL_COMPRESSED_RGBA_ARB.
		if (isCompressARBAvailable()) {
			// This texture contains mostly the clear color value and should therefore
			// compress well even with high quality.
			glHint(GL_TEXTURE_COMPRESSION_HINT_ARB, GL_NICEST);
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_COMPRESSED_RGBA_ARB, texturesize, texturesize, GL_RGBA, GL_BYTE, trackImage);
			// The following commented code is just for testing purposes.
			/*int compressed;
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed);
			if (compressed == GL_TRUE) {
				int csize;
				printf("compression succesful!\n");
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &csize);
				printf("compression ratio: %d to %d\n", csize, texturesize*texturesize*sizeof(GLuint));
			}*/
		} else {
			// GL_ARB_texture_compression not available at runtime, fallback.
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, texturesize, texturesize, GL_RGBA, GL_BYTE, trackImage);
		}

		// Free the memory of the initial texture.
		free(trackImage);

		// Init the position and size of the map in the window.
		map_x = -10;
		map_y = -40;
		map_size = 170;

		// Restore some state.
		glPopMatrix();

		// Compile the car "dot" display list.
		cardot = glGenLists(1);
		if (cardot != 0) {
			glNewList(cardot, GL_COMPILE);
			glBegin(GL_TRIANGLE_FAN);
			// The center.
			glVertex2f(0.0, 0.0);
			// The border.
			int i;
			const int borderpoints = 8;
			halflinewidth = halflinewidth*float(map_size)/float(texturesize);
			for (i = 0; i < borderpoints + 1; i++) {
				float phi = 2.0*PI*float(i)/float(borderpoints);
				glVertex2f(halflinewidth*cos(phi), halflinewidth*sin(phi));
			}
			glEnd();
			glEndList();
		}

		// Clear the screen, that in case of a delay the track is not visible.
		glClear(GL_COLOR_BUFFER_BIT);
	}
}


// Release the texture and data.
cGrTrackMap::~cGrTrackMap()
{
	// We have just one track texture object, so delete it just once.
	if (isinitalized) {
		glDeleteTextures(1, &mapTexture);
		isinitalized = false;
		if (cardot != 0) {
			glDeleteLists(cardot, 1);
		}
	}
}


// Walk trough the different available display modes
void cGrTrackMap::selectTrackMap()
{
	viewmode <<= 1;
	if (viewmode > TRACK_MAP_MASK) {
		viewmode = TRACK_MAP_NONE;
	}
}


// Draw the track map according to the display mode
void cGrTrackMap::display(

	tCarElt *currentCar,
	tSituation *situation,
	int Winx,
	int Winy,
	int Winw,
	int Winh
)
{

	if (viewmode == TRACK_MAP_NONE) {
		return;
	}

	// Compute track map position.
	int x = Winx + Winw + map_x - (int) (map_size*track_x_ratio);
	int y = Winy + Winh + map_y - (int) (map_size*track_y_ratio);

	// Setup and display track map.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, mapTexture);

	// Draw track.
	if (viewmode & (TRACK_MAP_NORMAL | TRACK_MAP_NORMAL_WITH_OPPONENTS)) {
		drawTrackNormal(x, y);
	} else if (viewmode & (TRACK_MAP_PAN | TRACK_MAP_PAN_WITH_OPPONENTS)) {
		drawTrackPanning(Winx, Winy, Winw, Winh, currentCar, situation);
	} else if (viewmode & (TRACK_MAP_PAN_ALIGNED | TRACK_MAP_PAN_ALIGNED_WITH_OPPONENTS)) {
		drawTrackPanningAligned(Winx, Winy, Winw, Winh, currentCar, situation);
	}

	// Draw cars in normal map mode.
	if (viewmode & TRACK_MAP_NORMAL_WITH_OPPONENTS) {
		drawCars(currentCar, situation, x, y);
	}
	if (viewmode & (TRACK_MAP_NORMAL | TRACK_MAP_NORMAL_WITH_OPPONENTS)) {
		drawCar(currentCar, currentCarColor, x, y);
	}

}


// Draw the track full visible and static.
void cGrTrackMap::drawTrackNormal(int x, int y)
{
	glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex2f(x, y);
    glTexCoord2f(1.0, 0.0); glVertex2f(x + map_size, y);
    glTexCoord2f(1.0, 1.0); glVertex2f(x + map_size, y + map_size);
    glTexCoord2f(0.0, 1.0); glVertex2f(x, y + map_size);
	glEnd();
}


// Draw the track in the panning mode.
void cGrTrackMap::drawTrackPanning(
	int Winx,
	int Winy,
	int Winw,
	int Winh,
	tCarElt *currentCar,
	tSituation *s
)
{
	float x1, y1, x2, y2;
	float tracksize = MAX(track_width, track_height);
	float radius = MIN(500.0, tracksize/2.0);
	x1 = (currentCar->_pos_X - radius - track_min_x)/tracksize;
	y1 = (currentCar->_pos_Y - radius - track_min_y)/tracksize;
	x2 = (currentCar->_pos_X + radius - track_min_x)/tracksize;
	y2 = (currentCar->_pos_Y + radius - track_min_y)/tracksize;

	// Draw track.
	int x = Winx + Winw + map_x - map_size;
	int y = Winy + Winh + map_y - map_size;
	glBegin(GL_QUADS);
    glTexCoord2f(x1, y1); glVertex2f(x, y);
    glTexCoord2f(x2, y1); glVertex2f(x + map_size, y);
    glTexCoord2f(x2, y2); glVertex2f(x + map_size, y + map_size);
    glTexCoord2f(x1, y2); glVertex2f(x, y + map_size);
	glEnd();

	// Draw car "dots".
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	if (viewmode & TRACK_MAP_PAN_WITH_OPPONENTS) {
		int i;
		for (i = 0; i < s->_ncars; i++) {
			if ((s->cars[i] != currentCar) &&
				!(s->cars[i]->_state &
				(RM_CAR_STATE_DNF | RM_CAR_STATE_PULLUP | RM_CAR_STATE_PULLSIDE | RM_CAR_STATE_PULLDN)))
			{
				if (s->cars[i]->race.pos > currentCar->race.pos) {
					glColor4fv(behindCarColor);
				} else {
					glColor4fv(aheadCarColor);
				}
				float xc = s->cars[i]->_pos_X - currentCar->_pos_X;
				float yc = s->cars[i]->_pos_Y - currentCar->_pos_Y;
				if (fabs(xc) < radius && fabs(yc) < radius) {
					xc = xc/radius*map_size;
					yc = yc/radius*map_size;

					glPushMatrix();
					glTranslatef(x + (xc + map_size)/2.0, y + (yc + map_size)/2.0, 0.0);
					float factor = tracksize/(2.0*radius);
		        	glScalef(factor, factor, 1.0);
					glCallList(cardot);
					glPopMatrix();
				}
			}
		}
	}

	glColor4fv(currentCarColor);
	if (cardot != 0) {
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(x + map_size/2.0, y + map_size/2.0, 0.0);
		float factor = tracksize/(2.0*radius);
		glScalef(factor, factor, 1.0);
		glCallList(cardot);
		glPopMatrix();
	}
}


// Draw the track in the panning aligned mode.
void cGrTrackMap::drawTrackPanningAligned(
	int Winx,
	int Winy,
	int Winw,
	int Winh,
	tCarElt *currentCar,
	tSituation *s
)
{
	float tracksize = MAX(track_width, track_height);
	float radius = MIN(500.0, tracksize/2.0);

	float x = Winx + Winw + map_x - map_size;
	float y = Winy + Winh + map_y - map_size;
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();

	glTranslatef(
		(currentCar->_pos_X - track_min_x)/tracksize,
		(currentCar->_pos_Y - track_min_y)/tracksize,
		0.0
	);
	glRotatef(currentCar->_yaw*360.0/(2.0*PI) - 90.0, 0.0, 0.0, 1.0);
	float factor = (2.0*radius)/tracksize;
	glScalef(factor, factor, 1.0);
	glTranslatef(-0.5, -0.5, 0.0);

	glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex2f(x, y);
    glTexCoord2f(1.0, 0.0); glVertex2f(x + map_size, y);
    glTexCoord2f(1.0, 1.0); glVertex2f(x + map_size, y + map_size);
    glTexCoord2f(0.0, 1.0); glVertex2f(x, y + map_size);
	glEnd();

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	// Draw car "dots".
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	if (viewmode & TRACK_MAP_PAN_ALIGNED_WITH_OPPONENTS) {
		int i;
		for (i = 0; i < s->_ncars; i++) {
			if ((s->cars[i] != currentCar) && !(s->cars[i]->_state & (RM_CAR_STATE_DNF | RM_CAR_STATE_PULLUP | RM_CAR_STATE_PULLSIDE | RM_CAR_STATE_PULLDN))) {
				if (s->cars[i]->race.pos > currentCar->race.pos) {
					glColor4fv(behindCarColor);
				} else {
					glColor4fv(aheadCarColor);
				}
				float xc = (s->cars[i]->_pos_X - currentCar->_pos_X)/(radius*2.0)*map_size;
				float yc = (s->cars[i]->_pos_Y - currentCar->_pos_Y)/(radius*2.0)*map_size;
				float ss = sin(-currentCar->_yaw + PI/2.0);
				float cs = cos(-currentCar->_yaw + PI/2.0);
				float xrc = xc * cs - yc * ss;
				float yrc = xc * ss + yc * cs;

				if (fabs(xrc) < map_size/2.0 && fabs(yrc) < map_size/2.0) {
					glPushMatrix();
					glTranslatef(x + xrc + map_size/2.0, y + yrc + map_size/2.0, 0.0);
					float factor = tracksize/(2.0*radius);
		        	glScalef(factor, factor, 1.0);
					glCallList(cardot);
					glPopMatrix();
				}
			}
		}
	}

	glColor4fv(currentCarColor);
	if (cardot != 0) {
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(x + map_size/2.0, y + map_size/2.0, 0.0);
		glScalef(1.0/factor, 1.0/factor, 1.0);
		glCallList(cardot);
		glPopMatrix();
	}
}


// Draw the dot of the car.
void cGrTrackMap::drawCar(tCarElt *currentCar, GLfloat* color, int x, int y)
{
	// Compute screen coordinates of the car.
	float car_x = (currentCar->_pos_X - track_min_x)/track_width*map_size*track_x_ratio + x;
	float car_y = (currentCar->_pos_Y - track_min_y)/track_height*map_size*track_y_ratio + y;

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glColor4fv(color);

	if (cardot != 0) {
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(car_x, car_y, 0.0);
		glCallList(cardot);
		glPopMatrix();
	}
}


// Draw all opponents of the current car.
void cGrTrackMap::drawCars(tCarElt *currentCar, tSituation *s, int x, int y)
{
	int i;
	for (i = 0; i < s->_ncars; i++) {
		if ((s->cars[i] != currentCar) &&
			!(s->cars[i]->_state &
			(RM_CAR_STATE_DNF | RM_CAR_STATE_PULLUP | RM_CAR_STATE_PULLSIDE | RM_CAR_STATE_PULLDN)))
		{
			if (s->cars[i]->race.pos > currentCar->race.pos) {
				drawCar(s->cars[i], behindCarColor, x, y);
			} else {
				drawCar(s->cars[i], aheadCarColor, x, y);
			}
		}
	}
}


// Checks if the new value is a minimum, and if true it assigns it
inline void cGrTrackMap::checkAndSetMinimum(float &currentmin, float &value)
{
	if (value < currentmin) {
		currentmin = value;
	}
	return;
}


// Checks if the new value is a maximum, and if true it assigns it
inline void cGrTrackMap::checkAndSetMaximum(float &currentmax, float &value)
{
	if (value > currentmax) {
		currentmax = value;
	}
	return;
}


// Setus up colors.
void cGrTrackMap::initColors()
{
	currentCarColor[0] = 1.0;
	currentCarColor[1] = 0.0;
	currentCarColor[2] = 0.0;
	currentCarColor[3] = 1.0;

	aheadCarColor[0] = 0.0;
	aheadCarColor[1] = 1.0;
	aheadCarColor[2] = 0.0;
	aheadCarColor[3] = 1.0;

	behindCarColor[0] = 0.0;
	behindCarColor[1] = 0.0;
	behindCarColor[2] = 1.0;
	behindCarColor[3] = 1.0;
}


// Set the view mode
void cGrTrackMap::setViewMode(int vm) {
    viewmode = vm;
}

// Get The view mode
int cGrTrackMap::getViewMode() {
    return viewmode;
}

// Get the default view mode
int cGrTrackMap::getDefaultViewMode() {
    return TRACK_MAP_NORMAL_WITH_OPPONENTS;
}
