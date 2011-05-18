/* console.c -- a simple console using cairo for rendering
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


#include "console.h"


/**
 * glyph_list operations
 */
glyph_list* new_glyph_node(FT_Face* font_face, wchar_t character)
{
  glyph_list* node = malloc(sizeof(glyph_list));
  node->previous = null;
  node->next = null;
  node->glyph = malloc(sizeof(cairo_glyph_t));
  node->character = character;
  node->glyph->index = FT_Get_Char_Index(*font_face, character);
  node->glyph->x = 0;
  node->glyph->y = 0;
  return node;
}
void delete_glyph_node(glyph_list* node)
{
  free(node->glyph);
  free(node);
}


/**
 * Initialize a new console
 */
Console* new_Console(cairo_t* context, double font_size)
{
  //printf("Console::Console(cairo_t@%p)\n", context);

  Console* self;
  self = malloc(sizeof(Console));
  self->head = null;
  self->tail = null;
  self->cursor = null;
  self->width = default_width;
  self->height = default_height;
  self->pad = 4;
  self->font_filename = "./Monaco_Linux.ttf"; /* TODO */
  self->font_size = font_size;
  self->transparency = 1.0;
  self->antialias_text = false;
  self->antialias_graphics = true;
  
  /* initialize lithp, cairo - and define callbacks */
  self->lithp = new_LithpInterpreter();
  char* cairo_init [] = {
    //"(set 'libcairo             (xl:dlopen \"libcairo.dylib\"))",
    //"(set 'libcairo             (xl:dlopen \"libcairo.so.2\"))",
    //"(set 'libcairo (xl:dlopen \"/Users/antoine/Projects/phlo/install/lib/libcairo.dylib\"))",
#if defined(__linux__)
    "(set 'libcairo             (xl:dlopen \"libcairo.so.2\"))",
#elif defined(__APPLE__) && defined(__MACH__)
    "(set 'libcairo             (xl:dlopen \"libcairo.dylib\"))",
#endif
    "(set 'cairo:set-source-rgb       (xl:dlsym libcairo \"cairo_set_source_rgb\"))",
    "(set 'cairo:rectangle            (xl:dlsym libcairo \"cairo_rectangle\"))",
    "(set 'cairo:fill                 (xl:dlsym libcairo \"cairo_fill\"))",
    "(set 'cairo:translate            (xl:dlsym libcairo \"cairo_translate\"))",
    "(set 'cairo:rotate               (xl:dlsym libcairo \"cairo_rotate\"))",
    "(set 'cairo:move-to              (xl:dlsym libcairo \"cairo_move_to\"))",
    "(set 'cairo:set-source-rgba      (xl:dlsym libcairo \"cairo_set_source_rgba\"))",
    "(set 'console:width 492)",
    "(set 'console:height 344)",
    "(set 'pi 3.141592)",

    "(set 'pretty '(lambda (cr arc theta)                      "
    "  (cond ((lte arc 0.0) 0.0)                               "
    "        ('t (progn                                        "
    "              (cairo:rotate cr theta)                     "
    "              (cairo:rectangle cr 20.0 20.0 140.0 20.0)    "
    "              (cairo:fill cr)                             "
    "              (pretty cr (sub arc theta) theta))))))      ",

    "(set 'console:expose-event '(lambda (context)             "
    "  (cairo:set-source-rgb  context 0.5 0.5 1.0)         "
    "  (cairo:translate context (div console:width 2.0) (div console:height 2.0))"
    "  (pretty context (mul pi 2.0) (div pi 4.0))))",
    //"(set 'console:expose-event '(lambda (context) ()))",
    null
  };
  for (size_t t = 0; cairo_init[t] != null; t++) {
    Expression* callback = Expression_parse_utf8(cairo_init[t], null, &self->lithp->symbol_list); 
    _gc_protect(callback);
    Expression_eval(callback, self->lithp->environment);
    _gc_unprotect(callback);
  }


  /* initialize and configure console font */
  FT_Library  library;
  int error = FT_Init_FreeType(&library);
  if (error) { perror("Could not open FreeType library"); exit(EXIT_FAILURE); }
  self->font_face = malloc(sizeof(FT_Face*));
  error = FT_New_Face(library, self->font_filename, 0, self->font_face);
  if (error) { perror("Could not open font"); exit(EXIT_FAILURE); }
  cairo_set_font_face(context, cairo_ft_font_face_create_for_ft_face(*self->font_face, 0));
  cairo_set_font_size(context, self->font_size);
  cairo_font_options_t* font_options = cairo_font_options_create();
  cairo_get_font_options(context, font_options);
  if (self->antialias_text) {
    cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_NONE);
  }
  cairo_set_font_options(context, font_options);
  self->font_extents = malloc(sizeof(cairo_font_extents_t));
  cairo_font_extents(context, self->font_extents);

  /* some sample text */
  //wchar_t* test_text = L"ABCDEFGHIJKLM\nNOPQRSTUVWXYZ\nabcdefghijklm\nnopqrstuvwxyz\n1234567890\0";
  wchar_t* test_text = L"  \n  ";
  unsigned int n;
  for (n = 0; n < wcslen(test_text); n++) {
    //  Console_append(self, test_text[n]);
    Console_insert(self, test_text[n]);
  }
  self->cursor = self->head;

  return self;
}


/**
 * Release memory and resources used by console
 */
void delete_Console(Console* self)
{
  free(self);
}


/** 
 * Append a character to the end of the console
 */
void Console_append(Console* self, wchar_t character)
{
  glyph_list* node = new_glyph_node(self->font_face, character);
  node->previous = self->tail;
  if (self->tail) {
    self->tail->next = node;
  }
  if (self->head == null) {
    self->head = node;
  } 
  self->tail = node;
  self->cursor = self->tail;
}


/** 
 * Insert a character at the current cursor position
 */
void Console_insert(Console* self, wchar_t character)
{
  if (self->cursor == self->tail) {
    return Console_append(self, character);
  }
  glyph_list* node = new_glyph_node(self->font_face, character);
  node->previous = self->cursor->previous;
  node->next = self->cursor;
  if (self->cursor == self->head) {
    self->head = node;
  } else {
    self->cursor->previous->next = node; 
  }
  self->cursor->previous = node;
}


/** 
 * Overwrite the character at the current cursor position
 */
void Console_overwrite(Console* self, wchar_t character)
{
}


/** 
 * Delete the characted at the current cursor position
 */
void Console_delete(Console* self)
{
  if ((self->cursor == null) || (self->cursor == self->tail)) {
    return;
  }
  glyph_list* node = self->cursor;
  if (node == self->head) {
    self->head = node->next;
  } else {
    node->previous->next = node->next;
  }
  node->next->previous = node->previous;
  self->cursor = node->next;
  delete_glyph_node(node);
}


/**
 * Layout console text
 */
void Console_layout(Console* self)
{
  if (self->head == null) {
    return;
  }
  glyph_list* current = self->head;
  current->glyph->x = 0;
  current->glyph->y = self->font_extents->ascent;
  current = current->next;
  while (current) {
    current->glyph->x = current->previous->glyph->x + self->font_extents->max_x_advance;
    current->glyph->y = current->previous->glyph->y;
    if ((current->glyph->x >= self->width - self->font_extents->max_x_advance) ||
        (current->previous->character == '\n')) {
      current->glyph->x = 0;
      current->glyph->y += self->font_extents->height;
    }
    current = current->next;
  }
}




/**
 * Handle console expose events
 */
void Console_expose(Console* self, cairo_t* context) 
{
  /* a pretty background */
  cairo_save(context);
  cairo_set_source_rgb(context, 1.0, 1.0, 1.0);
  cairo_translate(context, self->width / 2, self->height / 2);
  double t = pi / 4;
  for (double n = 0; n < pi * 2; n += t) {
    cairo_rectangle(context, 20, 20, 140, 40);
    cairo_fill(context);
    cairo_rotate(context, t);
  }
  cairo_restore(context);

  /* set console:width and console:height */
  char buf[256];
  sprintf(buf, "(set 'console:width %d)", (int)self->width);
  Expression* e = Expression_parse_utf8(buf, null, &self->lithp->symbol_list); 
  _gc_protect(e); Expression_eval(e, self->lithp->environment); _gc_unprotect(e);
  sprintf(buf, "(set 'console:height %d)", (int)self->height);
  e = Expression_parse_utf8(buf, null, &self->lithp->symbol_list); 
  _gc_protect(e); Expression_eval(e, self->lithp->environment); _gc_unprotect(e);


  /* invoke callbacks inside lithp */
  cairo_save(context);
  Expression* fn = Expression_intern("console:expose-event", &self->lithp->symbol_list);
  Expression* arg = _new_Expression(pointer);
  arg->car = (void*)context;
  _gc_protect(fn); _gc_protect(arg);
  Expression* callback = Expression_cons(fn, (Expression_cons(arg, nil_)));
  _gc_protect(callback);
  Expression_eval(callback, self->lithp->environment);
  _gc_unprotect(callback);
  _gc_unprotect(arg); _gc_unprotect(fn);
  cairo_restore(context);

  /* force lithp to garbage collect - TODO fix */
  self->lithp->gc->free_list = _gc_collect();

  /* select font */
  cairo_set_font_face(context, cairo_ft_font_face_create_for_ft_face(*self->font_face, 0));
  cairo_set_font_size(context, self->font_size);

  /* layout text */
  Console_layout(self);

  /* render text */  
  cairo_set_source_rgb(context, 0, 0, 0); 
  glyph_list* current = self->head;
  while (current) {
    cairo_show_glyphs(context, current->glyph, 1);
    current = current->next;
  }

  /* render cursor */
  cairo_set_source_rgba(context, 0, 0, 1, 0.5);
  double x = 0;
  double y = 0;
  if (self->cursor) {
    x = self->cursor->glyph->x; // + self->font_extents->max_x_advance;
    y = self->cursor->glyph->y - self->font_extents->ascent;
  }
  cairo_rectangle(context, x, y, self->font_extents->max_x_advance, self->font_extents->height);
  cairo_fill(context);
}


/**
 * Return the text between the two cursor positions
 */
#define code_buffer_size 1024
wchar_t code_buffer[code_buffer_size]; /* TODO */
wchar_t* Console_range_to_char(Console* self, glyph_list* start, glyph_list* end)
{
  if ((start == null) || (end == null) || (start == end)) {
    return null;
  } 
  memset(code_buffer, 0, code_buffer_size);
  size_t t = 0;
  for (glyph_list* i = start; i != end->next; i = i->next) {
    code_buffer[t] = i->character;
    code_buffer[t+1] = 0;
    t++;
  }
  return code_buffer;
}


/**
 * Handle console key-press events
 */
void Console_key_press(Console* self, wchar_t key) {
  Console_insert(self, key);
}
void Console_key_backspace(Console* self) {
  if (self->cursor && self->cursor->previous) {
    self->cursor = self->cursor->previous;
    Console_delete(self);
  }
}
void Console_key_delete(Console* self) {
  Console_delete(self);
}
void Console_key_left(Console* self) {
  if (self->cursor && self->cursor->previous) {
    self->cursor = self->cursor->previous;
  }
}
void Console_key_right(Console* self) {
  if (self->cursor && self->cursor->next) {
    self->cursor = self->cursor->next;
  }
}
void Console_key_up(Console* self) {
  if (self->cursor->glyph->y == 0) {
    return;
  }
  double x = self->cursor->glyph->x;
  double y = self->cursor->glyph->y;
  glyph_list* current = self->cursor;
  while (current) {
    if (x == current->glyph->x && y != current->glyph->y) {
      self->cursor = current;
      break;
    }
    current = current->previous;
  }
}
void Console_key_down(Console* self) {
  double x = self->cursor->glyph->x;
  double y = self->cursor->glyph->y;
  glyph_list* current = self->cursor;
  while (current) {
    if (x == current->glyph->x && y != current->glyph->y) {
      self->cursor = current;
      break;
    }
    current = current->next;
  }
}
void Console_key_execute(Console* self)
{
  glyph_list *start, *end;
  int open_bracket_count, close_bracket_count;
  bool have_token;
  wchar_t *code;
  Expression *expression, *ret;
  if (self->cursor->previous && self->cursor->character == '\n') { 
    end = self->cursor->previous; 
  } else {
    end = self->cursor;
  }
  while (end && (end != self->head)) {
    switch (end->character) {
    case '\n': printf("nada empty line\n"); return;
    case ' ': break;
    case '\t': break;
    default: goto have_end;
    }
    end = end->previous;
  }
 have_end:
  start = end; 
  open_bracket_count = 0;
  close_bracket_count = 0;
  have_token = false;
  while (start) { 
    switch (start->character) {
    case '(': 
      open_bracket_count++; 
      if (have_token && (close_bracket_count - open_bracket_count) == 0) goto have_start; 
      break;
    case ')': 
      if ((close_bracket_count - open_bracket_count) < 0) { printf("Nada 4\n"); return; }
      close_bracket_count++;
      if (have_token && (close_bracket_count - open_bracket_count) == 0) goto have_start; 
      break;
    case '\n':
      if (have_token && (close_bracket_count - open_bracket_count) == 0) goto have_start;
    default:
      have_token = true;
    }
    start = start->previous;
  }
 have_start:
  if ((close_bracket_count - open_bracket_count) != 0) {
    printf("nada brackets\n");
    return;
  } else if ((end - start) == 0) {
    printf("nada empty\n");
    return;
  } else  if (start == null) {
    start = self->head;
  }

  code = Console_range_to_char(self, start, end);
  if (code) { 
    expression = Expression_parse(code, wcslen(code), &self->lithp->symbol_list);
    if (expression == null || expression == nil_) { wprintf(L"Could not parse expression: %S\n", code); return; };
    printf("(EVAL '"); Expression_dump(expression, -1); printf(")\n");
    _gc_protect(expression);
    ret = Expression_eval(expression, self->lithp->environment);
    _gc_unprotect(expression);
    if (ret == null) { printf("Could not evaluate expression\n"); return; };
    wchar_t buf[1024]; 
    memset(buf, 0, 1024);
    Expression_to_string(ret, buf, 1024);
    printf("%S\n", buf);
    Console_insert(self, '\n');
    for (size_t t = 0; t < wcslen(buf); t++) {
      Console_insert(self, buf[t]);
    }
    Console_insert(self, '\n');
  }
}



/**
 * For testing bindings
 */
void Console_test(Console* self, int n)
{
  printf("Console::test(%p, %d)\n", self, n);
}
