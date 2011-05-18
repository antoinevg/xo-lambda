/* console.h -- a simple console using cairo for rendering
 *
 * Copyright (c) 2008 Antoine van Gelder
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the 'Software'),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, provided that the above copyright notice(s) and this
 * permission notice appear in all copies of the Software and that both the
 * above copyright notice(s) and this permission notice appear in supporting
 * documentation.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS'.  USE ENTIRELY AT YOUR OWN RISK.
 */

#ifndef CONSOLE_H
#define CONSOLE_H

/* wide-string handling */
#include <wchar.h>

/* gtk, freetype & cairo */
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <ft2build.h>
#include <freetype2/freetype/freetype.h>
#include <cairo.h>
#include <cairo-ft.h>

/* local includes */
#include <phlo.h>
#include <lithp.h>

#define pi 3.14159265359


/* defaults */
static const int default_width  = 492;
static const int default_height = 344;

/**
 * The console manages its text buffer with a doubly-linked list datastructure.
 *
 * This is that list.
 */
typedef struct _glyph_list {
  struct _glyph_list* previous;
  struct _glyph_list* next;
  wchar_t character;
  cairo_glyph_t* glyph;
} glyph_list;

glyph_list* new_glyph_node(FT_Face* font_face, wchar_t character);
void delete_glyph_node(glyph_list* node);


/** 
 * Console class definition
 */
typedef struct {
  /* properties */
  glyph_list* head;
  glyph_list* tail;
  glyph_list* cursor;
  double width;
  double height;
  double pad;
  double transparency;
  bool antialias_text;
  bool antialias_graphics;
  char* font_filename;
  double font_size;
  /* lithp interpreter */
  LithpInterpreter* lithp;
  /* state */
  FT_Face* font_face; /* needed the freetype face for converting char's to freetype glyph indices */
  cairo_font_extents_t* font_extents;
  cairo_t* cairo_context; 
} Console;

Console* new_Console(cairo_t* cairo_context, double font_size); /* TODO - can we lose the cairo_context ? */
void delete_Console(Console* self);
void Console_append(Console* self, wchar_t character);
void Console_insert(Console* self, wchar_t character);
void Console_overwrite(Console* self, wchar_t character);
void Console_delete(Console* self);
void Console_layout(Console* self);
void Console_expose(Console* self, cairo_t* context);
void Console_key_press(Console* self, wchar_t key);
void Console_key_backspace(Console* self);
void Console_key_delete(Console* self);
void Console_key_left(Console* self);
void Console_key_right(Console* self);
void Console_key_up(Console* self);
void Console_key_down(Console* self);
void Console_key_execute(Console* self);

void Console_test(Console* self, int n);

#endif /* CONSOLE_H */

