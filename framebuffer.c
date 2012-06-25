/**
  * GreenPois0n Cynanide - framebuffer.c
  * Copyright (C) 2010 Chronic-Dev Team
  * Copyright (C) 2010 Joshua Hill
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <string.h>

#include "font.h"
#include "device.h"
#include "common.h"
#include "commands.h"
#include "framebuffer.h"

static Font* gFbFont;
static unsigned int gFbX;
static unsigned int gFbY;
static unsigned int gFbTWidth;
static unsigned int gFbTHeight;
static unsigned int gFbWidth;
static unsigned int gFbHeight;

Bool gFbHasInit = FALSE;
Bool gFbDisplayText = FALSE;

static unsigned int gFbBackgroundColor;
static unsigned int gFbForegroundColor;

inline int font_get_pixel(Font* font, int ch, int x, int y) {
	register int index = ((font->width * font->height) * ch) + (font->width * y) + x;
	return (font->data[index / 8] >> (index % 8)) & 0x1;
}

volatile unsigned int* fb_get_pixel(register unsigned int x, register unsigned int y) {
	return (((unsigned int*)FRAMEBUFFER) + (y * gFbWidth) + x);
}

static void fb_scrollup() {
	register volatile unsigned int* newFirstLine = fb_get_pixel(0, gFbFont->height);
	register volatile unsigned int* oldFirstLine = fb_get_pixel(0, 0);
	register volatile unsigned int* end = oldFirstLine + (gFbWidth * gFbHeight);
	while(newFirstLine < end) {
		*(oldFirstLine++) = *(newFirstLine++);
	}
	while(oldFirstLine < end) {
		*(oldFirstLine++) = gFbBackgroundColor;
	}
	gFbY--;
}

void fb_setup() {
	gFbFont = (Font*) font_data;
	gFbBackgroundColor = COLOR_BLACK;
	gFbForegroundColor = COLOR_WHITE;
	gFbWidth = FRAMEBUFFER_WIDTH;
	gFbHeight = FRAMEBUFFER_HEIGHT;
	gFbTWidth = gFbWidth / gFbFont->width;
	gFbTHeight = gFbHeight / gFbFont->height;
}

int fb_init() {
	if(gFbHasInit) return 0;
	fb_setup();
	fb_clear();
	fb_set_loc(0,0);
	fb_display_text(TRUE);

	cmd_add("fbecho", &fb_cmd, "write characters back to framebuffer");
	cmd_add("fbclear", &fbclear_cmd, "clear the framebuffer");
	cmd_add("fbimg", &fbimg_cmd, "display image on framebuffer");
	cmd_add("fbprintcenter", &fbprintcenter_cmd, "write a character on the center of the framebuffer");
	cmd_add("fbprintln", &fbprintln_cmd, "write a character on the whole line");
	gFbHasInit = TRUE;
	return 0;
}

int fb_cmd(int argc, CmdArg* argv) {
	cmd_start();
	int i = 0;
	if (argc < 2) {
		puts("usage: fbecho <message>\n");
		return 0;
	}

	//enter_critical_section();
	for (i = 1; i < argc; i++) {
		fb_print(argv[i].string);
		fb_print(" ");
	}
	//exit_critical_section();
	fb_print("\n");
	return 0;
}

int fbimg_cmd(int argc, CmdArg* argv) {
	cmd_start();
	if (argc < 2) {
		puts("usage: fbimg <address>\n");
		return 0;
	}

	fb_draw_image((unsigned int*)argv[1].uinteger, 0, 0, gFbTWidth, gFbTHeight);
	return 0;
}

int fbprintcenter_cmd(int argc, CmdArg* argv) {
	cmd_start();
	if (argc < 2) {
		puts("usage: fbprintcenter <address>\n");
		return 0;
	}

	fb_print_center(argv[0]);
	return 0;
}

int fbprintln_cmd(int argc, CmdArg* argv) {
	cmd_start();
	if (argc < 2) {
		puts("usage: fbprintln <address>\n");
		return 0;
	}

	fb_print_line(argv[0].string);
	return 0;
}

int fbclear_cmd() {
	cmd_start();
	fb_setup();
	fb_clear();
	fb_set_loc(0,0);
	return 0;
}


void fb_clear() {
    unsigned int *p = 0;
	for(p = (unsigned int*)FRAMEBUFFER; p < (unsigned int*)(FRAMEBUFFER + (gFbWidth * gFbHeight * 4)); p++) {
        *p = gFbBackgroundColor;
    }
}

unsigned int fb_get_x() {
	return gFbX;
}

unsigned int fb_get_y() {
	return gFbY;
}

unsigned int fb_get_width() {
	return gFbTWidth;
}

unsigned int fb_get_height() {
	return gFbTHeight;
}

void fb_set_loc(unsigned int x, unsigned int y) {
	gFbX = x;
	gFbY = y;
}

void fb_display_text(Bool option) {
	gFbDisplayText = option;
}

void fb_putc(int c) {
	if(c == '\r') {
		gFbX = 0;

	} else if(c == '\n') {
		gFbX = 0;
		gFbY++;

	} else {
		register unsigned int sx;
		register unsigned int sy;

		for(sy = 0; sy < gFbFont->height; sy++) {
			for(sx = 0; sx < gFbFont->width; sx++) {
				if(font_get_pixel(gFbFont, c, sx, sy)) {
					*(fb_get_pixel(sx + (gFbFont->width * gFbX), sy + (gFbFont->height * gFbY))) = gFbForegroundColor;
				} else {
					*(fb_get_pixel(sx + (gFbFont->width * gFbX), sy + (gFbFont->height * gFbY))) = gFbBackgroundColor;
				}
			}
		}

		gFbX++;
	}

	if(gFbX == gFbTWidth) {
		gFbX = 0;
		gFbY++;
	}

	if(gFbY == gFbTHeight) {
		fb_scrollup();
	}
}

void fb_print_line(char c) {
	int i;
	for(i=0;i<gFbTWidth;i++) {
		fb_putc(c);
	}
}

void fb_print_center(const char* str) {
	int i;
	int pad = (gFbTWidth-strlen(str))/2;
	for(i=0;i<pad;i++) {
		fb_putc(' ');
	}
	fb_print(str);
}

void fb_print(const char* str) {
	if(!gFbDisplayText)
		return;

	unsigned int len = strlen(str);
	int i;
	for(i = 0; i < len; i++) {
		fb_putc(str[i]);
	}
}

void fb_print_force(const char* str) {
	size_t len = strlen(str);
	int i;
	for(i = 0; i < len; i++) {
		fb_putc(str[i]);
	}
}

void fb_draw_image(unsigned int* image, unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
	register unsigned int sx;
	register unsigned int sy;
	for(sy = 0; sy < height; sy++) {
		for(sx = 0; sx < width; sx++) {
			*(fb_get_pixel(sx + x, sy + y)) = image[(sy * width) + sx];
		}
	}
}

void fb_capture_image(unsigned int* image, unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
	register unsigned int sx;
	register unsigned int sy;
	for(sy = 0; sy < height; sy++) {
		for(sx = 0; sx < width; sx++) {
			image[(sy * width) + sx] = *(fb_get_pixel(sx + x, sy + y));
		}
	}
}
