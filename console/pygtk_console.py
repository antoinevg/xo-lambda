#!/usr/bin/env python2.5

import sys
sys.path.append('.')

import pygtk
pygtk.require('2.0')
import gtk
import gtk.gdk

import cairo
import ctypes
class PycairoContext(ctypes.Structure):
    _fields_ = [("PyObject_HEAD", ctypes.c_byte * object.__basicsize__),
                ("ctx", ctypes.c_void_p),
                ("base", ctypes.c_void_p)]
import console
default_width = 492
default_height = 344


class GtkLambda:

    def __init__(self):
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.set_resizable(True)
        self.window.set_app_paintable(True)
        # self.window.set_icon_from_file("lambda-console.png")
        self.window.set_title("lambda-console")
        self.window.set_default_size(default_width, default_height)
        self.window.show_all()
        #context = gtk.gdk.cairo_create(self.window)
        context = self.window.window.cairo_create()
        cairo_t = PycairoContext.from_address(id(context)).ctx
        self.console = console.Console(cairo_t, 10.0)
        self.window.connect("delete_event", gtk.main_quit)
        self.window.connect("key-press-event", self.key_press_event)
        self.window.connect("expose-event", self.expose_event)
        self.window.show()

    def key_press_event(self, widget, event):
        if event.type is not gtk.gdk.KEY_PRESS:
            print "Not a gtk.gdk.KEY_PRESS"
            return
        keyval = event.keyval
        if keyval == gtk.keysyms.Escape:
            gtk.main_quit()
        elif keyval == gtk.keysyms.Return:
            keyval = ord('\n') 
        elif keyval == gtk.keysyms.Shift_L:
            return False
        elif keyval == gtk.keysyms.Shift_R:
            return False
        elif keyval == gtk.keysyms.BackSpace:
            self.console.key_backspace()
        elif keyval == gtk.keysyms.Delete:
            self.console.key_delete()
        elif keyval == gtk.keysyms.Left:
            self.console.key_left()
        elif keyval == gtk.keysyms.Right:
            self.console.key_right()
        elif keyval == gtk.keysyms.Up:
            self.console.key_up()
        elif keyval == gtk.keysyms.Down:
            self.console.key_down()
        if keyval >= 0 and keyval <= 255:
            self.console.key_press(keyval)
        self.window.queue_draw()
        return False

    def expose_event(self, widget, event):
        self.console.width  = widget.allocation.width
        self.console.height = widget.allocation.height
        context = widget.window.cairo_create()

        # TODO - set console width
        context.set_source_rgb(1, 1, 1)
        context.paint()
        # TODO - transparency & anti-alasing options
        context.save()
        cairo_t = PycairoContext.from_address(id(context)).ctx
        self.console.expose(cairo_t)
        context.restore()
        return False

    def main(self):
        gtk.main()

    def destroy(self, widget, data=None):
        gtk.main_quit()



if __name__ == "__main__":
    gtk_lambda = GtkLambda()
    gtk_lambda.main()
