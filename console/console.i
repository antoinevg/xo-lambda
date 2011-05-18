%module console
%{
  #include "console.h"
%}

/* There _has_ to be a cleaner way to do this! */
/* TODO - take a PyCairoContext or whatever pycairo wraps cairo_t in directly */
%typemap(in) cairo_t * {
  $1 = (cairo_t*)((unsigned int)PyInt_AsLong($input));
}  

typedef struct {
  double width;
  double height;

  %extend {
    Console(cairo_t* cairo_context, double font_size);
    ~Console();

    void expose(cairo_t* cairo_context);
    void key_backspace();
    void key_delete();
    void key_left();
    void key_right();
    void key_up();
    void key_down();
    void key_press(unsigned int key); /* TODO - swig no likee wchar_t either */
    void key_execute();

    void test(int n); 
  }

} Console;

