
/* gcc -o test_cairo test-cairo.c `pkg-config cairo --cflags --libs` */


#include <stdio.h>
#include <cairo.h>


int main(int argc, char** argv) 
{
  cairo_surface_t* surface;
  cairo_t* cr;

  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 400, 400);
  cr = cairo_create(surface);
  cairo_set_line_width(cr, 15);
  cairo_move_to(cr, 200, 100);
  cairo_line_to(cr, 300, 300);
  cairo_rel_line_to(cr, 50, 0);
  cairo_close_path(cr);
  cairo_stroke(cr);

  cairo_surface_write_to_png(surface, "triangle-c.png");

  cairo_destroy(cr);
  cairo_surface_destroy(surface);

  return 0;
}
