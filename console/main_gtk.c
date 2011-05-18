/* main_gtk.c -- Sample GTK host for console
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


/**
 * Sample GTK Host for console
 *
 * Any toolkit which plays nicely with cairo should be compatible with the
 * console code. 
 *
 * The basic procedure consists of:
 *   1. Create a top-level window which contains a cairo surface
 *   2. Pass the cairo context for that window to the Console constructor
 *   3. Wire up input device events to the appropriate Console_key_*() methods
 *   4. Wire up the expose or draw event for your top-level window to the Console_expose() method.
 */

#include <console.h>

/* forward decls */
static gboolean key_press_event(GtkWidget* widget, GdkEventKey* event, Console* console);
static void expose_event(GtkWidget* widget, GdkEventExpose* event, Console* console);


/**
 * Program entry-point
 */
gint main(gint argc, gchar** argv)
{
  GtkWidget* window;
  Console* console;

  gtk_init(&argc, &argv);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_resizable(GTK_WINDOW(window), true);
  gtk_widget_set_app_paintable(window, true);
  //gtk_window_set_icon_from_file(GTK_WINDOW(window), "lambda-console.png", null);
  gtk_window_set_title(GTK_WINDOW(window), "λ-console");
  gtk_window_set_default_size(GTK_WINDOW(window), default_width, default_height);
  gtk_widget_show_all(window);

  console = new_Console(gdk_cairo_create(window->window), 10.0);

  g_signal_connect(G_OBJECT(window), "delete-event",    G_CALLBACK(gtk_main_quit),   null);
  g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(key_press_event), console);
  g_signal_connect(G_OBJECT(window), "expose-event",    G_CALLBACK(expose_event),    console);

  gtk_main();

  return EXIT_SUCCESS;
}


/** 
 * Handle GTK key-press-events
 */
static bool extended_mode = 0;
static gboolean key_press_event(GtkWidget* widget, GdkEventKey* event, Console* console)
{
  if (event->type == GDK_KEY_PRESS) {
    if (event->state & GDK_CONTROL_MASK) {
      switch (event->keyval) {
      case 'x': extended_mode = true; break;
      case 'e': if (extended_mode) Console_key_execute(console); break; 
      case GDK_Return: Console_key_execute(console); break; 
      case 'g': extended_mode = false; break;
      default: 
        if (extended_mode) {
          printf("C-x C-%c is undefined\n", event->keyval);
        }
        extended_mode = false;
      }
      gtk_widget_queue_draw(widget);
      return false;
    }

    switch (event->keyval) {
    case GDK_Escape:    gtk_main_quit(); return false;
    case GDK_Return:    event->keyval = '\n'; break; 
    case GDK_Shift_L:   return false;
    case GDK_Shift_R:   return false;
    case GDK_BackSpace: Console_key_backspace(console); break;
    case GDK_Delete:    Console_key_delete(console); break;
    case GDK_Left:      Console_key_left(console); break;
    case GDK_Right:     Console_key_right(console); break;
    case GDK_Up:        Console_key_up(console); break;
    case GDK_Down:      Console_key_down(console); break;
    }

    if (event->keyval >= 0 && event->keyval <= 255) {
      Console_key_press(console, event->keyval);
    }
    
    gtk_widget_queue_draw(widget);
  }
  return false;
}


/**
 * Handle GTK expose-events
 */
static void expose_event(GtkWidget* widget, GdkEventExpose* event, Console* console)
{
  console->width = widget->allocation.width;
  console->height = widget->allocation.height;

  cairo_t* context = gdk_cairo_create(widget->window);

  /* background */
  cairo_set_source_rgba(context, 1, 1, 1, console->transparency);
  cairo_paint(context);

  /* options */
  if (console->transparency < 1.0) {
    cairo_set_operator(context, CAIRO_OPERATOR_SOURCE); 
  }
  if (console->antialias_graphics == true) {
    cairo_set_antialias(context, CAIRO_ANTIALIAS_DEFAULT);
  } else {
    cairo_set_antialias(context, CAIRO_ANTIALIAS_NONE);
  }

  /* draw the console */
  cairo_save(context);
  Console_expose(console, context); 
  cairo_restore(context);
  
  cairo_destroy(context);
}


